//
// Copyright (c) 2008-2020 the Urho3D project.
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

#pragma once

#include "Sample.h"

namespace Urho3D
{
    class Node;
    class Scene;
    class RigidBody2D;
}

class Ball2D;
class BallSucker;

/// Urho2D physics sample.
/// This sample demonstrates:
///     - Creating both static and moving 2D physics objects to a scene
///     - Displaying physics debug geometry
class Balls2DPhysics : public Sample
{
    URHO3D_OBJECT(Balls2DPhysics, Sample);

public:
    /// Construct.
    explicit Balls2DPhysics(Context* context);

    /// Setup after engine initialization and before running the main loop.
    void Start() override;

protected:
    /// Return XML patch instructions for screen joystick layout for a specific sample app, if any.
    String GetScreenJoystickPatchString() const override { return
        "<patch>"
        "    <remove sel=\"/element/element[./attribute[@name='Name' and @value='Button0']]/attribute[@name='Is Visible']\" />"
        "    <replace sel=\"/element/element[./attribute[@name='Name' and @value='Button0']]/element[./attribute[@name='Name' and @value='Label']]/attribute[@name='Text']/@value\">Zoom In</replace>"
        "    <add sel=\"/element/element[./attribute[@name='Name' and @value='Button0']]\">"
        "        <element type=\"Text\">"
        "            <attribute name=\"Name\" value=\"KeyBinding\" />"
        "            <attribute name=\"Text\" value=\"PAGEUP\" />"
        "        </element>"
        "    </add>"
        "    <remove sel=\"/element/element[./attribute[@name='Name' and @value='Button1']]/attribute[@name='Is Visible']\" />"
        "    <replace sel=\"/element/element[./attribute[@name='Name' and @value='Button1']]/element[./attribute[@name='Name' and @value='Label']]/attribute[@name='Text']/@value\">Zoom Out</replace>"
        "    <add sel=\"/element/element[./attribute[@name='Name' and @value='Button1']]\">"
        "        <element type=\"Text\">"
        "            <attribute name=\"Name\" value=\"KeyBinding\" />"
        "            <attribute name=\"Text\" value=\"PAGEDOWN\" />"
        "        </element>"
        "    </add>"
        "</patch>";
    }

private:
    /// Construct the scene content.
    void CreateScene();
    /// Construct an instruction text to the UI.
    void CreateInstructions();

    void CreateWalls();

    void CreateSucker();

    void CreateRacket(const Vector2 &pos);

    void CreateBall();

    void CreateSink();

    void CreateEditor();

    void CreateScore();
    /// Set up a viewport for displaying the scene.
    void SetupViewport();
    /// Subscribe to application-wide logic update events.
    void SubscribeToEvents();

    void DebugDraw();
    /// Handle the logic update event.
    void HandleSceneUpdate(StringHash eventType, VariantMap& eventData);

    void HandlePostUpdate(StringHash eventType, VariantMap& eventData);

    void HandleMouseButtonDown(StringHash eventType, VariantMap& eventData);

    void HandleMouseButtonUp(StringHash eventType, VariantMap& eventData);

    void HandleMouseMove(StringHash eventType, VariantMap& eventData);

    void HandleTouchBegin3(StringHash eventType, VariantMap& eventData);

    void HandleTouchMove3(StringHash eventType, VariantMap& eventData);

    void HandleTouchEnd3(StringHash eventType, VariantMap& eventData);

    void HandlePhysicsBegin2D(StringHash eventType, VariantMap& eventData);

    void HandleKeyDown(StringHash eventType, VariantMap& eventData);

    Vector2 GetMousePositionXY();

    void TestNodes();

    Vector<Color> colors_;

    Camera* camera_;

    Node* pickedNode_;

    Node* tailNode_;

    Node* sinkNode_;

    Node* textNode_;

    Node* spriterNode_;

    Node* ballsNode_;

    Node* ballSuckerNode_;

    Node* ballRacketNode_;

    RigidBody2D* dummyBody_;

    Text* timerText_;

    Rect field_;

    float ballTimer_;

    float scalePhysics_;

    bool debugDraw_;
};
