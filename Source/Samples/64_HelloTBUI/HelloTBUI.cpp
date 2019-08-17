//
// Copyright (c) 2008-2016 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/CheckBox.h>
#include <Urho3D/UI/LineEdit.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/ToolTip.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/TBElement.h>
#include <Urho3D/UI/ImGuiElement.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/UI/Window.h>

#include "HelloTBUI.h"

#include <Urho3D/DebugNew.h>

URHO3D_DEFINE_APPLICATION_MAIN(HelloTBUI)

class CheckPointWindow : public TBWindow, public Object
{
	URHO3D_OBJECT(CheckPointWindow, Object)
public:

	TBOBJECT_SUBCLASS(CheckPointWindow, TBWindow)

	CheckPointWindow(TBWidget* root, Context* context) 
		: Object(context)
	{};

	~CheckPointWindow() {};
};

HelloTBUI::HelloTBUI(Context* context) :
    Sample(context),
    uiRoot_(GetSubsystem<UI>()->GetRoot()),
    dragBeginPosition_(IntVector2::ZERO)
{
}

void HelloTBUI::Start()
{
    // Execute base class startup
    Sample::Start();

    // Enable OS cursor
    GetSubsystem<Input>()->SetMouseVisible(true);

    // Load XML file containing default UI style sheet
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* style = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
	Urho3D::Graphics* g = GetSubsystem<Urho3D::Graphics>();

    // Set the loaded style as default style
    uiRoot_->SetDefaultStyle(style);
    uiRoot_->SetSize(800, 600);

	//ImGuiElement* imgui = new ImGuiElement(context_);
	//ImGuiElement::RegisterObject(context_);
	//imgui->SetPosition(0, 0);
	//imgui->SetSize(800, 600);

	// ImGuiElement* imgui2 = new ImGuiElement(context_);
	// imgui->AddChild(imgui2);

	//uiRoot_->AddChild(imgui);

	//ToolTip* toolTip = new ToolTip(context_);
	//imgui->AddChild(toolTip);

    tbelement = new TBUIElement(context_);
    TBUIElement::RegisterObject(context_);
    tbelement->SetPosition(0, 0);

	// tbelement->SetSize(g->GetWidth(), g->GetHeight());
    tbelement->SetBoxSize(g->GetWidth(), g->GetHeight());
    tbelement->SetAlignment(HA_CENTER, VA_BOTTOM);

    TBRootWidget* stateUI = new TBRootWidget(context_);
    stateUI->SetGravity(WIDGET_GRAVITY_ALL);
    tbelement->AddStateWidget(stateUI, true);
    tbelement->LoadResources();
    tbelement->LoadWidgets(stateUI, "Data/TB/layout/debug_screen.txt");

    uiRoot_->AddChild(tbelement);

	SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(HelloTBUI, HandleKeyDown));

    // Initialize Window
    // InitWindow();

    // Create and add some controls to the Window
    //InitControls();

    // Create a draggable Fish
    // CreateDraggableFish();

    // Set the mouse mode to use in the sample
    Sample::InitMouseMode(MM_FREE);
}

void HelloTBUI::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
	int key = eventData[KeyDown::P_KEY].GetInt();

	if (key == KEY_F1)
		GetSubsystem<Console>()->Toggle();

	if (key == KEY_F4)
	{
#ifdef TB_RUNTIME_DEBUG_INFO
		ShowDebugInfoSettingsWindow(tbelement->GetRoot());
#else
		URHO3D_LOGERRORF("gamestate.handlekeydown: TB_RUNTIME_DEBUG_INFO not defined");
#endif
	}
}

void HelloTBUI::InitControls()
{
    // Create a CheckBox
    CheckBox* checkBox = new CheckBox(context_);
    checkBox->SetName("CheckBox");

    // Create a Button
    Button* button = new Button(context_);
    button->SetName("Button");
    button->SetMinHeight(24);

    // Create a LineEdit
    LineEdit* lineEdit = new LineEdit(context_);
    lineEdit->SetName("LineEdit");
    lineEdit->SetMinHeight(24);

    // Add controls to Window
    window_->AddChild(checkBox);
    window_->AddChild(button);
    window_->AddChild(lineEdit);

    // Apply previously set default style
    checkBox->SetStyleAuto();
    button->SetStyleAuto();
    lineEdit->SetStyleAuto();
}

void HelloTBUI::InitWindow()
{
    // Create the Window and add it to the UI's root node
    window_ = new Window(context_);
    uiRoot_->AddChild(window_);

    // Set Window size and layout settings
    window_->SetMinWidth(684);
	window_->SetPosition(0,0);
    window_->SetLayout(LM_VERTICAL, 6, IntRect(6, 6, 6, 6));
    window_->SetAlignment(HA_CENTER, VA_CENTER);
    window_->SetName("Window");

    // Create Window 'titlebar' container
    UIElement* titleBar = new UIElement(context_);
    titleBar->SetMinSize(0, 24);
    titleBar->SetVerticalAlignment(VA_TOP);
    titleBar->SetLayoutMode(LM_HORIZONTAL);

    // Create the Window title Text
    Text* windowTitle = new Text(context_);
    windowTitle->SetName("WindowTitle");
    windowTitle->SetText("Hello GUI!");

    // Create the Window's close button
    Button* buttonClose = new Button(context_);
    buttonClose->SetName("CloseButton");

    // Add the controls to the title bar
    titleBar->AddChild(windowTitle);
    titleBar->AddChild(buttonClose);

    // Add the title bar to the Window
    window_->AddChild(titleBar);

    // Apply styles
    window_->SetStyleAuto();
    windowTitle->SetStyleAuto();
    buttonClose->SetStyle("CloseButton");
    
    // Subscribe to buttonClose release (following a 'press') events
    SubscribeToEvent(buttonClose, E_RELEASED, URHO3D_HANDLER(HelloTBUI, HandleClosePressed));

    // Subscribe also to all UI mouse clicks just to see where we have clicked
    SubscribeToEvent(E_UIMOUSECLICK, URHO3D_HANDLER(HelloTBUI, HandleControlClicked));
}

void HelloTBUI::CreateDraggableFish()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    Graphics* graphics = GetSubsystem<Graphics>();

    // Create a draggable Fish button
    Button* draggableFish = new Button(context_);
    draggableFish->SetTexture(cache->GetResource<Texture2D>("Textures/UrhoDecal.dds")); // Set texture
    draggableFish->SetBlendMode(BLEND_ADD);
    draggableFish->SetSize(128, 128);
    draggableFish->SetPosition((graphics->GetWidth() - draggableFish->GetWidth()) / 2, 200);
    draggableFish->SetName("Fish");
    uiRoot_->AddChild(draggableFish);

    // Add a tooltip to Fish button
    //ToolTip* toolTip = new ToolTip(context_);
    //draggableFish->AddChild(toolTip);
    //toolTip->SetPosition(IntVector2(draggableFish->GetWidth() + 5, draggableFish->GetWidth() / 2)); // slightly offset from close button
    //BorderImage* textHolder = new BorderImage(context_);
    //toolTip->AddChild(textHolder);
    //textHolder->SetStyle("ToolTipBorderImage");
    //Text* toolTipText = new Text(context_);
    //textHolder->AddChild(toolTipText);
    //toolTipText->SetStyle("ToolTipText");
    //toolTipText->SetText("Please drag me!");

    // Subscribe draggableFish to Drag Events (in order to make it draggable)
    // See "Event list" in documentation's Main Page for reference on available Events and their eventData
    SubscribeToEvent(draggableFish, E_DRAGBEGIN, URHO3D_HANDLER(HelloTBUI, HandleDragBegin));
    SubscribeToEvent(draggableFish, E_DRAGMOVE, URHO3D_HANDLER(HelloTBUI, HandleDragMove));
    SubscribeToEvent(draggableFish, E_DRAGEND, URHO3D_HANDLER(HelloTBUI, HandleDragEnd));
}

void HelloTBUI::HandleDragBegin(StringHash eventType, VariantMap& eventData)
{
    // Get UIElement relative position where input (touch or click) occurred (top-left = IntVector2(0,0))
    dragBeginPosition_ = IntVector2(eventData["ElementX"].GetInt(), eventData["ElementY"].GetInt());
}

void HelloTBUI::HandleDragMove(StringHash eventType, VariantMap& eventData)
{
    IntVector2 dragCurrentPosition = IntVector2(eventData["X"].GetInt(), eventData["Y"].GetInt());
    UIElement* draggedElement = static_cast<UIElement*>(eventData["Element"].GetPtr());
    draggedElement->SetPosition(dragCurrentPosition - dragBeginPosition_);
}

void HelloTBUI::HandleDragEnd(StringHash eventType, VariantMap& eventData) // For reference (not used here)
{
}

void HelloTBUI::HandleClosePressed(StringHash eventType, VariantMap& eventData)
{
    if (GetPlatform() != "Web")
        engine_->Exit();
}

void HelloTBUI::HandleTBUIReleased(StringHash eventType, VariantMap& eventData)
{
    URHO3D_LOGINFOF("HelloTBUI::HandleTBUIReleased");
    tbelement->Clear();
}

void HelloTBUI::HandleControlClicked(StringHash eventType, VariantMap& eventData)
{
    // Get the Text control acting as the Window's title
    Text* windowTitle = window_->GetChildStaticCast<Text>("WindowTitle", true);

    // Get control that was clicked
    UIElement* clicked = static_cast<UIElement*>(eventData[UIMouseClick::P_ELEMENT].GetPtr());

    String name = "...?";
    if (clicked)
    {
        // Get the name of the control that was clicked
        name = clicked->GetName();
    }

    // Update the Window's title text
    windowTitle->SetText("Hello " + name + "!");
}
