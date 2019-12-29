#include "../UI/TBElement.h"

#include "../Graphics/Graphics.h"
#include "../Graphics/GraphicsEvents.h"
#include "../Graphics/Texture2D.h"
#include "../IO/Log.h"
#include "../IO/File.h"
#include "../IO/FileSystem.h"
#include "../Core/CoreEvents.h"
#include "../Core/ProcessUtils.h"
#include "../Input/Input.h"
#include "../UI/UI.h"
#include "../UI/UIEvents.h"

#include "tb_core.h"
#include "tb_system.h"
#include "tb_widgets.h"
#include "tb_select.h"
#include "tb_font_renderer.h"
#include "tb_language.h"
#include "tb_widgets_reader.h"
#include "animation/tb_animation.h"
#include "animation/tb_widget_animation.h"
#include "tb_node_tree.h"
#include "tb_editfield.h"
#include "tb_menu_window.h"

#include <SDL/SDL.h>

#define VERTEX_SIZE 6

static const int KEY_QUAL_OFFSET = 0x8000;
/*
 Cosas a resolver
 - el Context global no es una buena idea.
 - crear tantas texturas tampoco es buena idea.
 - el teclado tiene que andar en android
        - la solucion es medio chota
 */


// Register tbbf font renderer
#ifdef TB_FONT_RENDERER_TBBF
    void register_tbbf_font_renderer();
//    register_tbbf_font_renderer();
#endif

static Context *g_context = nullptr;
static String g_programDir;

namespace Urho3D
{

extern const char* UI_CATEGORY;

/*------------ TBBitmapUrho3D ------------*/
TBBitmapUrho3D::TBBitmapUrho3D(TBRendererUrho3D* renderer)
    : renderer_(renderer),
      width_(0),
      height_(0),
      texture_(nullptr)
{
    texture_ = new Texture2D(g_context);

    texture_->SetMipsToSkip(QUALITY_LOW, 0);
    texture_->SetNumLevels(1);
}

TBBitmapUrho3D::~TBBitmapUrho3D()
{
    renderer_->FlushBitmap(this);
}

bool TBBitmapUrho3D::Init(int width, int height, uint32 *data)
{
    width_ = width;
    height_ = height;

    SetData(data);
    return true;
}

void TBBitmapUrho3D::SetData(uint32 *data)
{
    if(!texture_->SetSize(width_, height_, Graphics::GetRGBAFormat()))
        URHO3D_LOGINFOF("tex <%p> error setsize!", texture_.Get());

    if(!texture_->SetData(0, 0, 0, width_, height_, data))
        URHO3D_LOGINFOF("tex <%p> error setdata!", texture_.Get());

    texture_->SetAddressMode(COORD_U, ADDRESS_WRAP);
    texture_->SetAddressMode(COORD_V, ADDRESS_WRAP); 
}

/*------------ TBRendererUrho3D ------------*/
TBRendererUrho3D::TBRendererUrho3D()
{
}

TBRendererUrho3D::~TBRendererUrho3D()
{
}

void TBRendererUrho3D::BeginPaint(int render_target_w, int render_target_h)
{
    // URHO3D_LOGERRORF("---------- begin paint -----------");
    TBRendererBatcher::BeginPaint(render_target_w, render_target_h);

    // clipRect_.Set(clipRect_.x, clipRect_.y, render_target_w, render_target_h);
}

void TBRendererUrho3D::EndPaint()
{
    TBRendererBatcher::EndPaint();
    // URHO3D_LOGERRORF("---------- end paint -----------");
}

TBBitmap *TBRendererUrho3D::CreateBitmap(int width, int height, uint32 *data)
{
    TBBitmapUrho3D* bitmap = new TBBitmapUrho3D(this);

//     URHO3D_LOGINFOF("create bitmap <%p> w,h,d <%u,%u,%p>", bitmap, width, height,data);
    bitmap->Init(width, height, data);

//     FlushBitmap((TBBitmap*)bitmap);

    return (TBBitmap*)bitmap;
}

// TB render batch call.
void TBRendererUrho3D::RenderBatch(TBRendererBatcher::Batch *batch)
{
    TBBitmapUrho3D* bitmap = (TBBitmapUrho3D*)(batch->bitmap);
    SharedPtr<Texture2D> dummy;
    SharedPtr<Texture2D> texture = bitmap ? bitmap->GetTexture() : dummy;

    IntRect scissor(clipRect_.x, clipRect_.y, clipRect_.x + clipRect_.w, clipRect_.y + clipRect_.h);
    UIBatch uiBatch(0, BLEND_ALPHA, scissor, texture, &vertexData_);

    // color 4 unsigned byte (o 1 float)
    // uv coord 2 float
    // point 2 float
    // resize vertex data
    unsigned vertexSize = VERTEX_SIZE * batch->vertex_count;
    uiBatch.vertexStart_ = vertexData_.Size();
    uiBatch.vertexEnd_ = uiBatch.vertexStart_ + vertexSize;

    vertexData_.Resize(uiBatch.vertexStart_ + vertexSize);
    float* dest = &(vertexData_.At(uiBatch.vertexStart_));

    unsigned offset = 0;
    for(int i = 0; i < batch->vertex_count; i++, offset += VERTEX_SIZE)
    {
        TBRendererBatcher::Vertex v = batch->vertex[i];

        dest[offset + 0] = batch->vertex[i].x;
        dest[offset + 1] = batch->vertex[i].y;
        dest[offset + 2] = 0.0f;
        ((unsigned&)dest[offset + 3]) = (unsigned)batch->vertex[i].col;
        dest[offset + 4] = batch->vertex[i].u;
        dest[offset + 5] = batch->vertex[i].v;
    }

    UIBatch::AddOrMerge(uiBatch, batches_);
}

void TBRendererUrho3D::SetClipRect(const TBRect &rect)
{
    clipRect_ = rect;
}

void TBRendererUrho3D::Clear()
{
    batches_.Clear();
    vertexData_.Clear();
}

/*------------ TBElement ------------*/
TBUIElement::TBUIElement(Context* context)
    : UIElement(context)
{
    SetName("TBUIElement");

    SetEnabled(true);
    SetFocusMode(FM_FOCUSABLE);

    renderer_ = new TBRendererUrho3D();
    if (!tb_core_is_initialized())
    {
        tb_core_init(renderer_);
    }

    root_ = new TBRootWidget(context);
    root_->SetAutoFocusState(true);

    CreateKeyMap();

    SubscribeToEvent(E_SCREENMODE, URHO3D_HANDLER(TBUIElement, HandleScreenMode));
    SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(TBUIElement, HandlePostUpdate));

    SubscribeToEvent(E_BEGINFRAME, URHO3D_HANDLER(TBUIElement, HandleBeginFrame));

//    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(TBUIElement, HandleKeyDown));
//    SubscribeToEvent(E_KEYUP, URHO3D_HANDLER(TBUIElement, HandleKeyUp));

    SubscribeToEvent(E_SDLRAWINPUT, URHO3D_HANDLER(TBUIElement, HandleRawEvent));

//    SubscribeToEvent(this, E_FOCUSED, URHO3D_HANDLER(TBUIElement, HandleFocused));
//    SubscribeToEvent(this, E_DEFOCUSED, URHO3D_HANDLER(LineEdit, HandleDefocused));
}

TBUIElement::~TBUIElement()
{
    if (tb_core_is_initialized())
    {
        tb_core_shutdown();
    }

    if(renderer_)
    {
        delete renderer_;
        renderer_ = nullptr;
    }
}

void TBUIElement::RegisterObject(Context* context)
{
    g_context = context;

    FileSystem fs(context);
    g_programDir = fs.GetProgramDir();

    context->RegisterFactory<TBUIElement>(UI_CATEGORY);
}

void TBUIElement::SetBoxSize(int width, int height)
{
    root_->SetSize(width, height);
    SetSize(width, height);
}

void TBUIElement::AddStateWidget(TBWidget* stateWidget, bool bottom, bool fullscreen)
{
    if(fullscreen)
        stateWidget->SetSize(root_->GetRect().w, root_->GetRect().h);

    root_->AddChild(stateWidget, bottom ? WIDGET_Z_BOTTOM : WIDGET_Z_TOP);
}

void TBUIElement::LoadWidgets(TBWidget* stateWidget, String filename)
{
    if(!g_widgets_reader->LoadFile(stateWidget, filename.CString()))
    {
        URHO3D_LOGERRORF("cannot load <%s>", filename.CString());
    }
}

void TBUIElement::LoadResources()
{
    TBWidgetListener::AddGlobalListener(root_);

    if(!g_tb_lng->Load("Data/TB/language/lng_en.tb.txt"))
    {
        URHO3D_LOGINFO("cannot load language");
    }

    // Load the default skin, and override skin that contains the graphics specific to the demo.
    // , "Data/TB/demo_skin/skin.tb.txt")
    if(!g_tb_skin->Load("Data/TB/skin/skin.tb.txt", "Data/TB/skin/skin.sk.txt"))
        URHO3D_LOGINFO("cannot load skin");

#ifdef TB_FONT_RENDERER_TBBF
    register_tbbf_font_renderer();
#endif

    // Add a font to the font manager.
    g_font_manager->AddFontInfo("Data/TB/font/segoe_white_with_shadow.tb.txt", "Segoe");
    g_font_manager->AddFontInfo("Data/TB/font/neon.tb.txt", "Neon");

    // Set the default font description for widgets to one of the fonts we just added
    TBFontDescription fd;
    fd.SetID(TBIDC("Segoe"));
    fd.SetSize(g_tb_skin->GetDimensionConverter()->DpToPx(14));
    g_font_manager->SetDefaultFontDescription(fd);

    // Create the font now.
    TBFontFace *font = g_font_manager->CreateFontFace(g_font_manager->GetDefaultFontDescription());

    // Give the root widget a background skin
    root_->SetSkinBg("background_solid");

    TBWidgetsAnimationManager::Init();
}

void TBUIElement::Clear()
{
    root_->GetContentRoot()->DeleteAllChildren();
}

void TBUIElement::GetBatches(PODVector<UIBatch>& batches, PODVector<float>& vertexData, const IntRect& currentScissor)
{
    float colorStep = 359.0f / renderer_->GetBatchSize();
    Color color;

    for(int i = 0; i < renderer_->GetBatchSize(); i++)
    {
        UIBatch& batch = renderer_->GetBatch(i);
        batch.element_ = this;
//        batch.scissor_.top_ += screenPosition_.y_;
//        batch.scissor_.left_ += screenPosition_.x_ - 500;
        // batch.scissor_.bottom_ += screenPosition_.y_;

        unsigned batchStart = batch.vertexStart_;
        unsigned batchEnd = batch.vertexEnd_;
        unsigned batchSize = batch.vertexEnd_ - batch.vertexStart_;

        // resize dest data vertex vector
        unsigned begin = vertexData.Size();
        unsigned newSize = begin + batchSize;

        vertexData.Resize(newSize);
        float* dest = &(vertexData.At(begin));

        // batch.vertexStart_ = begin;
        // batch.vertexEnd_ = vertexData.Size();

        // float* src = &(batch.vertexData_->At(batchStart));
        // PODVector<float>* ptrVec = batch.vertexData_;
        // URHO3D_LOGERRORF(" src <%p> pointer <%p>", src, &(*batch.vertexData_)[batchStart]);
        // memcpy(&vertexData[begin], &(*batch.vertexData_)[batchStart], batchSize * sizeof(float));
        //memcpy(dest, src, batchSize * sizeof(float));

        unsigned offset = 0;
        #define VER_COL(r, g, b, a) (((a)<<24) + ((b)<<16) + ((g)<<8) + r)
        for(unsigned j = batchStart; j < batchEnd; j += VERTEX_SIZE)
        {
//            dest[offset + 0] += screenPosition_.x_;
//            dest[offset + 1] += screenPosition_.y_;

//            dest[offset + 0] = batch.vertexData_->At(j + 0) + screenPosition_.x_;
//            dest[offset + 1] = batch.vertexData_->At(j + 1) + screenPosition_.y_;
            dest[offset + 0] = batch.vertexData_->At(j + 0);
            dest[offset + 1] = batch.vertexData_->At(j + 1);
            dest[offset + 2] = batch.vertexData_->At(j + 2);
            float col   = batch.vertexData_->At(j + 3);
            color.FromHSL(colorStep * i, 100.0f, 50.0f);
            // unsigned col = color.ToUInt();
            ((unsigned&)dest[offset + 3]) = (unsigned&)col;
            dest[offset + 4] = batch.vertexData_->At(j + 4);
            dest[offset + 5] = batch.vertexData_->At(j + 5);

            offset += VERTEX_SIZE;
        }

        UIBatch::AddOrMerge(batch, batches);
    }

    renderer_->Clear();
}

int mouse_x = 0;
int mouse_y = 0;
bool key_alt = false;
bool key_ctrl = false;
bool key_shift = false;
bool key_super = false;

MODIFIER_KEYS GetModifierKeys()
{
    MODIFIER_KEYS code = TB_MODIFIER_NONE;
    if (key_alt)    code |= TB_ALT;
    if (key_ctrl)   code |= TB_CTRL;
    if (key_shift)  code |= TB_SHIFT;
    if (key_super)  code |= TB_SUPER;
    return code;
}

static bool ShouldEmulateTouchEvent()
{
    // Used to emulate that mouse events are touch events when alt, ctrl and shift are pressed.
    // This makes testing a lot easier when there is no touch screen around :)
    return (GetModifierKeys() & (TB_ALT | TB_CTRL | TB_SHIFT)) ? true : false;
}

void TBUIElement::CreateKeyMap()
{
    mapKey_.Insert(Pair<int, int>(KEY_UP, TB_KEY_UP));
    mapKey_.Insert(Pair<int, int>(KEY_DOWN, TB_KEY_DOWN));
    mapKey_.Insert(Pair<int, int>(KEY_LEFT, TB_KEY_LEFT));
    mapKey_.Insert(Pair<int, int>(KEY_RIGHT, TB_KEY_RIGHT));
    mapKey_.Insert(Pair<int, int>(KEY_BACKSPACE, TB_KEY_BACKSPACE));
    mapKey_.Insert(Pair<int, int>(KEY_TAB, TB_KEY_TAB));
    mapKey_.Insert(Pair<int, int>(KEY_INSERT, TB_KEY_INSERT));
    mapKey_.Insert(Pair<int, int>(KEY_DELETE, TB_KEY_DELETE));
    mapKey_.Insert(Pair<int, int>(KEY_PAGEUP, TB_KEY_PAGE_UP));
    mapKey_.Insert(Pair<int, int>(KEY_PAGEDOWN, TB_KEY_PAGE_DOWN));
    mapKey_.Insert(Pair<int, int>(KEY_HOME, TB_KEY_HOME));
    mapKey_.Insert(Pair<int, int>(KEY_END, TB_KEY_END));
    mapKey_.Insert(Pair<int, int>(KEY_RETURN, TB_KEY_ENTER));
    mapKey_.Insert(Pair<int, int>(KEY_ESCAPE, TB_KEY_ESC));
    mapKey_.Insert(Pair<int, int>(KEY_F1, TB_KEY_F1));
    mapKey_.Insert(Pair<int, int>(KEY_F2, TB_KEY_F2));
    mapKey_.Insert(Pair<int, int>(KEY_F3, TB_KEY_F3));
    mapKey_.Insert(Pair<int, int>(KEY_F4, TB_KEY_F4));
    mapKey_.Insert(Pair<int, int>(KEY_F5, TB_KEY_F5));
    mapKey_.Insert(Pair<int, int>(KEY_F6, TB_KEY_F6));
    mapKey_.Insert(Pair<int, int>(KEY_F7, TB_KEY_F7));
    mapKey_.Insert(Pair<int, int>(KEY_F8, TB_KEY_F8));
    mapKey_.Insert(Pair<int, int>(KEY_F9, TB_KEY_F9));
    mapKey_.Insert(Pair<int, int>(KEY_F10, TB_KEY_F10));
    mapKey_.Insert(Pair<int, int>(KEY_F11, TB_KEY_F11));
    mapKey_.Insert(Pair<int, int>(KEY_F12, TB_KEY_F12));

    mapKey_.Insert(Pair<int, int>(QUAL_SHIFT + KEY_QUAL_OFFSET, TB_SHIFT));
    mapKey_.Insert(Pair<int, int>(QUAL_CTRL + KEY_QUAL_OFFSET, TB_CTRL));
    mapKey_.Insert(Pair<int, int>(QUAL_ALT+ KEY_QUAL_OFFSET, TB_ALT));
    mapKey_.Insert(Pair<int, int>(QUAL_ANY + KEY_QUAL_OFFSET, TB_SUPER));
}

int TBUIElement::FindKeyMap(int key)
{
    auto it = mapKey_.Find(key);
    if (it != mapKey_.End())
        return it->second_;

    return TB_KEY_UNDEFINED;
}

void TBUIElement::OnClickBegin (const IntVector2& position, const IntVector2& screenPosition, int button, int buttons, int qualifiers, Cursor* cursor)
{
    root_->InvokePointerDown(position.x_, position.y_, 0, GetModifierKeys(), ShouldEmulateTouchEvent());
}

void TBUIElement::OnClickEnd (const IntVector2& position, const IntVector2& screenPosition, int button, int buttons, int qualifiers, Cursor* cursor, UIElement* beginElement)
{
    root_->InvokePointerUp(position.x_, position.y_, GetModifierKeys(), ShouldEmulateTouchEvent());
}

void TBUIElement::OnWheel(int delta, MouseButtonFlags buttons, QualifierFlags qualifiers)
{
    root_->InvokeWheel(mouse_x, mouse_y, 0, -delta, GetModifierKeys());
}

void TBUIElement::OnPositionSet(const IntVector2& newPosition)
{
    int posx = newPosition.x_;
    int posy = newPosition.y_;

    root_->SetPosition(TBPoint(posx, posy));

//    TBRect rect = renderer_->GetClipRect();
//    renderer_->SetClipRect(TBRect(posx, posy, posx + rect.w, posy + rect.h));

    // Urho3D::Graphics* g = GetSubsystem<Urho3D::Graphics>();
    // renderer_->SetClipRect(TBRect(0, 0, g->GetWidth(), g->GetHeight()));
}

void TBUIElement::OnResize(const IntVector2& newSize, const IntVector2& delta)
{
    root_->SetSize(newSize.x_, newSize.y_);
}

bool TBUIElement::IsWithinScissor(const IntRect& currentScissor)
{
    return true;
}

const IntVector2& TBUIElement::GetScreenPosition() const
{
    UIElement::GetScreenPosition();
    root_->SetPosition(TBPoint(screenPosition_.x_, screenPosition_.y_));
    return screenPosition_;
}

void TBUIElement::Update(float timeStep)
{
    double t = TBMessageHandler::GetNextMessageFireTime();

    //     if (t != TB_NOT_SOON && t <= TBSystem::GetTimeMS())
//     {
//         URHO3D_LOGINFOF("update ProcessMessages");
//         TBMessageHandler::ProcessMessages();
//     }
    TBMessageHandler::ProcessMessages();
}

void TBUIElement::HandlePostUpdate(StringHash eventType, VariantMap& eventData)
{
    Urho3D::Graphics* g = GetSubsystem<Urho3D::Graphics>();

    //renderer_->BeginPaint(GetWidth(), GetHeight());
    // renderer_->BeginPaint(root_->GetRect().w, root_->GetRect().h);
    renderer_->BeginPaint(g->GetWidth(), g->GetHeight());

    root_->InvokePaint(TBWidget::PaintProps());

    if (TBAnimationManager::HasAnimationsRunning())
    {
        root_->Invalidate();
    }
    renderer_->EndPaint();
}

void TBUIElement::HandleBeginFrame(StringHash eventType, VariantMap& eventData)
{
    TBAnimationManager::Update();

    root_->InvokeProcessStates();
    root_->InvokeProcess();
}

void TBUIElement::HandleScreenMode(StringHash eventType, VariantMap& eventData)
{
//    using namespace ScreenMode;
//    int w = eventData[P_WIDTH].GetInt();
//    int h = eventData[P_HEIGHT].GetInt();
}

void TBUIElement::HandleFocused(StringHash /*eventType*/, VariantMap& eventData)
{
    if (GetSubsystem<UI>()->GetUseScreenKeyboard())
        GetSubsystem<Input>()->SetScreenKeyboardVisible(true);
}

void TBUIElement::HandleKeyDown(StringHash eventType, VariantMap &eventData)
{
    using namespace KeyDown;
    unsigned qualifiers = eventData[P_QUALIFIERS].GetUInt();
    unsigned key = eventData[P_KEY].GetUInt();

    int tbModifier = FindKeyMap(KEY_QUAL_OFFSET + qualifiers);
    int tbSpecial = FindKeyMap(key);
    if (tbSpecial != TB_KEY_UNDEFINED)
        key = TB_KEY_UNDEFINED;

    root_->InvokeKey(key, (SPECIAL_KEY)tbSpecial, (MODIFIER_KEYS)tbModifier, true);
}

void TBUIElement::HandleKeyUp(StringHash eventType, VariantMap &eventData)
{
    using namespace KeyDown;
    unsigned qualifiers = eventData[P_QUALIFIERS].GetUInt();
    unsigned key = eventData[P_KEY].GetUInt();

    int tbModifier = FindKeyMap(KEY_QUAL_OFFSET + qualifiers);
    int tbSpecial = FindKeyMap(key);

    root_->InvokeKey(key, (SPECIAL_KEY)tbSpecial, (MODIFIER_KEYS)tbModifier, false);
}

void TBUIElement::HandleRawEvent(StringHash eventType, VariantMap& args)
{
    auto event = static_cast<SDL_Event*>(args[SDLRawInput::P_SDLEVENT].Get<void*>());

//    if (event->type != SDL_MOUSEMOTION && event->type != SDL_CONTROLLERAXISMOTION && event->type != SDL_JOYAXISMOTION)
//        URHO3D_LOGERRORF("tbuielement.handleeventraw: event <%i>", event->type);

    // intercept mouse event to move window and not forwarding to scene
    if (event->type == SDL_MOUSEMOTION)
    {
        int x = event->motion.x;
        int y = event->motion.y;

        mouse_x = x;
        mouse_y = y;

        TBWidget *widget;
        if (widget = root_->GetWidgetAt(x, y, true))
        {
            // root_->InvokePointerMove(x, y, GetModifierKeys(), ShouldEmulateTouchEvent());
            widget->ConvertFromRoot(x, y);
            TBWidgetEvent ev(EVENT_TYPE_POINTER_MOVE, x, y, ShouldEmulateTouchEvent(), GetModifierKeys());
            if (widget->InvokeEvent(ev))
            {
                args[SDLRawInput::P_CONSUMED] = true;
                return;
            }
        }
    }
    else if (event->type == SDL_CONTROLLERDEVICEADDED || event->type == SDL_JOYDEVICEADDED)
    {
        if (event->type == SDL_CONTROLLERDEVICEADDED)
        {
            URHO3D_LOGERRORF("tbuielement.handleeventraw: controller added <%i>", event->cdevice.which);
        }
        else if (event->type == SDL_JOYDEVICEADDED)
        {
            URHO3D_LOGERRORF("tbuielement.handleeventraw: joystick added <%i>", event->jdevice.which);
        }
    }
    else if (event->type == SDL_CONTROLLERDEVICEREMOVED || event->type == SDL_JOYDEVICEREMOVED)
    {
        if (event->type == SDL_CONTROLLERDEVICEREMOVED)
        {
            URHO3D_LOGERRORF("tbuielement.handleeventraw: controller removed <%i>", event->cdevice.which);
        }
        else if (event->type == SDL_JOYDEVICEREMOVED)
        {
            URHO3D_LOGERRORF("tbuielement.handleeventraw: controller removed <%i>", event->jdevice.which);
        }
    }
    else if (event->type == SDL_CONTROLLERDEVICEREMAPPED)
    {
        URHO3D_LOGERRORF("tbuielement.handleeventraw: controller remapped");
    }

    // FIXME this is navigation control, but should not have effect while focus is on
    // edit alike field
    if (event->type == SDL_JOYHATMOTION)
    {
        int key = 9;
        SPECIAL_KEY special = TB_KEY_TAB;

        if (event->jhat.value & SDL_HAT_RIGHT)
        {
            root_->InvokeKey(key, special, TB_MODIFIER_NONE, true);

            //URHO3D_LOGERRORF("TBUIElement::HandleRawEvent: type <%u> hat <%u> axis value <%i>",
            //    event->type, event->jhat, event->jaxis.value);
        }
        else if (event->jhat.value & SDL_HAT_LEFT)
        {
            root_->InvokeKey(key, special, TB_SHIFT, true);

            //URHO3D_LOGERRORF("TBUIElement::HandleRawEvent: type <%u> hat <%u> axis value <%i>",
            //    event->type, event->jhat, event->jaxis.value);
        }
    }

    //if (event->type == SDL_JOYAXISMOTION)
    //{
    //    int key = 9;
    //    SPECIAL_KEY special = TB_KEY_TAB;

    //    if (event->jaxis.value == 32767)
    //    {
    //        root_->InvokeKey(key, special, TB_MODIFIER_NONE, true);

    //        //URHO3D_LOGERRORF("TBUIElement::HandleRawEvent: type <%u> hat <%u> axis value <%i>",
    //        //    event->type, event->jhat, event->jaxis.value);
    //    }
    //    else if (event->jaxis.value == -32768)
    //    {
    //        root_->InvokeKey(key, special, TB_SHIFT, true);

    //        //URHO3D_LOGERRORF("TBUIElement::HandleRawEvent: type <%u> hat <%u> axis value <%i>",
    //        //    event->type, event->jhat, event->jaxis.value);
    //    }
    //}

    //if (event->type == SDL_JOYBUTTONDOWN)
    //{
    //    int key = 13;
    //    root_->InvokeKey(key, TB_KEY_ENTER, TB_MODIFIER_NONE, true);
    //}
    //else if (event->type == SDL_JOYBUTTONUP)
    //{
    //    int key = 13;
    //    root_->InvokeKey(key, TB_KEY_ENTER, TB_MODIFIER_NONE, false);
    //}
}

TBRootWidget::TBRootWidget(Context* context)
    : Urho3D::Object(context)
{
}

bool TBRootWidget::OnEvent(const TBWidgetEvent &ev)
{
    if(ev.type == EVENT_TYPE_CLICK)
    {
        Urho3D::VariantMap eventData;
        eventData[P_BUTTON_ID] = ev.target->GetID();
        eventData[P_BUTTON_TEXT] = ev.target->GetText().CStr();

        TBEditField *edit = TBSafeCast<TBEditField>(ev.target);
        if(edit && !edit->GetReadOnly())
            GetSubsystem<Input>()->SetScreenKeyboardVisible(true);

        SendEvent(E_TBUI_BUTTON_RELEASED, eventData);

        return true;
    }
    else if(ev.type == EVENT_TYPE_CHANGED)
    {
        Urho3D::VariantMap eventData;
        eventData[P_WIDGET_ID] =  ev.target->GetID();
        eventData[P_WIDGET_VALUE] = ev.target->GetValueDouble();

        SendEvent(E_TBUI_WIDGET_CHANGED, eventData);
        return true;
    }

    return false;
}

void TBRootWidget::OnWidgetFocusChanged(TBWidget *widget, bool focused)
{
    URHO3D_LOGERRORF("TBRootWidget::OnWidgetFocusChanged: widget <%s> edit focus <%u>", widget->GetID().debug_string.CStr(), focused);
    // esto es para desplegar el teclado en android
    // cuando un editfield obtiene el foco
    if (TBEditField *edit = TBSafeCast<TBEditField>(widget))
    {
        // URHO3D_LOGERRORF("TBRootWidget::OnWidgetFocusChanged: edit focus <%u>", focused);
        if (!edit->GetReadOnly())
        {
            GetSubsystem<Input>()->SetScreenKeyboardVisible(focused);
        }
    }
}

} // end namespace Urho3D

namespace tb 
{

class TBUrho3DFile : public TBFile
{

public:
    TBUrho3DFile(File* f) : file(f) {}
    virtual ~TBUrho3DFile() { file->Close(); }

    virtual long Size()
    {
        return file->GetSize();
    }
    virtual size_t Read(void *buf, size_t elemSize, size_t count)
    {
        return file->Read(buf, elemSize * count);
    }
private:
    Urho3D::File *file;
};

TBFile *TBFile::Open(const char *filename, TBFileMode mode)
{
    Urho3D::File *f = 0;
    String filepath = g_programDir + filename;
    // URHO3D_LOGINFOF("TBFile::Open: filepath <%s>", filepath.CString());
    switch (mode)
    {
        case MODE_READ:
            f = new File(g_context, filepath, FILE_READ);
            break;
        default:
            break;
    }
    if (!f)
        return NULL;

    TBUrho3DFile *tbf = new TBUrho3DFile(f);
    if(!tbf)
        delete f;
    return tbf;
}

} // end namespace tb 

