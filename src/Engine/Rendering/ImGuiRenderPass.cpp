#include "Rendering/ImGuiRenderPass.h"

ImGuiRenderPass::ImGuiRenderPass(IRenderer* renderer, EventBroker* eventBroker)
    : m_Renderer(renderer)
    , m_EventBroker(eventBroker)
{
    g_Window = renderer->Window();

    ImGuiIO& io = ImGui::GetIO();
    io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;                         // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
    io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
    io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
    io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
    io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
    io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
    io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
    io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
    io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
    io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

    ImGuiStyle& style = ImGui::GetStyle();
    style.Alpha = 1.f;
    style.WindowPadding = ImVec2(8.f, 7.f);
    style.WindowRounding = 4.f;
    style.ChildWindowRounding = 0.f;
    style.FramePadding = ImVec2(4.f, 2.f);
    style.FrameRounding = 2.f;
    style.ItemSpacing = ImVec2(6.f, 2.f);
    style.ItemInnerSpacing = ImVec2(3.f, 4.f);
    style.IndentSpacing = 16.f;
    style.ScrollbarSize = 12;
    style.ScrollbarRounding = 2.f;
    style.GrabMinSize = 13.f;
    style.GrabRounding = 3.f;

    createDeviceObjects();
    
    EVENT_SUBSCRIBE_MEMBER(m_EMousePress, &ImGuiRenderPass::OnMousePress);
    EVENT_SUBSCRIBE_MEMBER(m_EMouseRelease, &ImGuiRenderPass::OnMouseRelease);
    EVENT_SUBSCRIBE_MEMBER(m_EMouseMove, &ImGuiRenderPass::OnMouseMove);
    EVENT_SUBSCRIBE_MEMBER(m_EMouseScroll, &ImGuiRenderPass::OnMouseScroll);
    EVENT_SUBSCRIBE_MEMBER(m_EKeyDown, &ImGuiRenderPass::OnKeyDown);
    EVENT_SUBSCRIBE_MEMBER(m_EKeyUp, &ImGuiRenderPass::OnKeyUp);
    EVENT_SUBSCRIBE_MEMBER(m_EKeyboardChar, &ImGuiRenderPass::OnKeyboardChar);

    // Prime the first frame
    newFrame();
}

void ImGuiRenderPass::Update(double dt)
{
    g_DeltaTime = dt;
}

void ImGuiRenderPass::Draw()
{
    ImGuiIO& io = ImGui::GetIO();

    ImGui::Render();

    ImDrawData* draw_data = ImGui::GetDrawData();

    // Set up render state
    ImGuiRenderState state;
    glActiveTexture(GL_TEXTURE0);

    // Handle cases of screen coordinates != from framebuffer coordinates (e.g. retina displays)
    float fb_height = io.DisplaySize.y * io.DisplayFramebufferScale.y;
    draw_data->ScaleClipRects(io.DisplayFramebufferScale);

    // Setup viewport, orthographic projection matrix
    glViewport(0, 0, (GLsizei)io.DisplaySize.x, (GLsizei)io.DisplaySize.y);
    const float ortho_projection[4][4] =
    {
        { 2.0f/io.DisplaySize.x, 0.0f, 0.0f, 0.0f },
        { 0.0f, 2.0f/-io.DisplaySize.y, 0.0f, 0.0f },
        { 0.0f, 0.0f, -1.0f, 0.0f },
        { -1.0f, 1.0f, 0.0f, 1.0f },
    };
    glUseProgram(g_ShaderHandle);
    glUniform1i(g_AttribLocationTex, 0);
    glUniformMatrix4fv(g_AttribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]);
    glBindVertexArray(g_VaoHandle);

    for (int n = 0; n < draw_data->CmdListsCount; n++) {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const ImDrawIdx* idx_buffer_offset = 0;

        glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.size() * sizeof(ImDrawVert), (GLvoid*)&cmd_list->VtxBuffer.front(), GL_STREAM_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ElementsHandle);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.size() * sizeof(ImDrawIdx), (GLvoid*)&cmd_list->IdxBuffer.front(), GL_STREAM_DRAW);

        for (const ImDrawCmd* pcmd = cmd_list->CmdBuffer.begin(); pcmd != cmd_list->CmdBuffer.end(); pcmd++) {
            if (pcmd->UserCallback) {
                pcmd->UserCallback(cmd_list, pcmd);
            } else {
                glBindTexture(GL_TEXTURE_2D, (GLuint)pcmd->TextureId);
                glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
                glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
            }
            idx_buffer_offset += pcmd->ElemCount;
        }
    }

    // Start next frame
    newFrame();
}

bool ImGuiRenderPass::OnMousePress(const Events::MousePress& e)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MouseDown[e.Button] = true;
    return false;
}

bool ImGuiRenderPass::OnMouseRelease(const Events::MouseRelease& e)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MouseDown[e.Button] = false;
    return false;
}

bool ImGuiRenderPass::OnMouseMove(const Events::MouseMove& e)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos.x = e.X;
    io.MousePos.y = e.Y;
    return true;
}

bool ImGuiRenderPass::OnMouseScroll(const Events::MouseScroll& e)
{
    g_MouseWheel += (float)e.DeltaY;
    return true;
}

bool ImGuiRenderPass::OnKeyDown(const Events::KeyDown& e)
{
    ImGuiIO& io = ImGui::GetIO();
    io.KeysDown[e.KeyCode] = true;
    return true;
}

bool ImGuiRenderPass::OnKeyUp(const Events::KeyUp& e)
{
    ImGuiIO& io = ImGui::GetIO();
    io.KeysDown[e.KeyCode] = false;
    return true;
}

bool ImGuiRenderPass::OnKeyboardChar(const Events::KeyboardChar& e)
{
    ImGuiIO& io = ImGui::GetIO();
    if (e.Char > 0 && e.Char < 0x10000) {
        io.AddInputCharacter((unsigned short)e.Char);
        return true;
    } else {
        return false;
    }
}

bool ImGuiRenderPass::createDeviceObjects()
{
    // Backup GL state
    GLint last_texture, last_array_buffer, last_vertex_array;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);

    const GLchar *vertex_shader =
        "#version 330\n"
        "uniform mat4 ProjMtx;\n"
        "in vec2 Position;\n"
        "in vec2 UV;\n"
        "in vec4 Color;\n"
        "out vec2 Frag_UV;\n"
        "out vec4 Frag_Color;\n"
        "void main()\n"
        "{\n"
        "	Frag_UV = UV;\n"
        "	Frag_Color = Color;\n"
        "	gl_Position = ProjMtx * vec4(Position.xy, 0, 1);\n"
        "}\n";

    const GLchar* fragment_shader =
        "#version 330\n"
        "uniform sampler2D Texture;\n"
        "in vec2 Frag_UV;\n"
        "in vec4 Frag_Color;\n"
        "out vec4 Out_Color;\n"
        "void main()\n"
        "{\n"
        "	Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
        "}\n";

    g_ShaderHandle = glCreateProgram();
    g_VertHandle = glCreateShader(GL_VERTEX_SHADER);
    g_FragHandle = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(g_VertHandle, 1, &vertex_shader, 0);
    glShaderSource(g_FragHandle, 1, &fragment_shader, 0);
    glCompileShader(g_VertHandle);
    glCompileShader(g_FragHandle);
    glAttachShader(g_ShaderHandle, g_VertHandle);
    glAttachShader(g_ShaderHandle, g_FragHandle);
    glLinkProgram(g_ShaderHandle);

    g_AttribLocationTex = glGetUniformLocation(g_ShaderHandle, "Texture");
    g_AttribLocationProjMtx = glGetUniformLocation(g_ShaderHandle, "ProjMtx");
    g_AttribLocationPosition = glGetAttribLocation(g_ShaderHandle, "Position");
    g_AttribLocationUV = glGetAttribLocation(g_ShaderHandle, "UV");
    g_AttribLocationColor = glGetAttribLocation(g_ShaderHandle, "Color");

    glGenBuffers(1, &g_VboHandle);
    glGenBuffers(1, &g_ElementsHandle);

    glGenVertexArrays(1, &g_VaoHandle);
    glBindVertexArray(g_VaoHandle);
    glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
    glEnableVertexAttribArray(g_AttribLocationPosition);
    glEnableVertexAttribArray(g_AttribLocationUV);
    glEnableVertexAttribArray(g_AttribLocationColor);

#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
    glVertexAttribPointer(g_AttribLocationPosition, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, pos));
    glVertexAttribPointer(g_AttribLocationUV, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, uv));
    glVertexAttribPointer(g_AttribLocationColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, col));
#undef OFFSETOF

    createFontsTexture();

    // Restore modified GL state
    glBindTexture(GL_TEXTURE_2D, last_texture);
    glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
    glBindVertexArray(last_vertex_array);

    return true;
}

bool ImGuiRenderPass::createFontsTexture()
{
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();

    io.Fonts->AddFontFromFileTTF("Fonts/DroidSans.ttf", 13.f);
    //io.Fonts->AddFontFromFileTTF("Fonts/ProggyClean.ttf", 13.f);
    //io.Fonts->AddFontFromFileTTF("Fonts/ProggyTiny.ttf", 10.f);
    //io.Fonts->AddFontFromFileTTF("Fonts/Karla-Regular.ttf", 15.0f);

    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bits for OpenGL3 demo because it is more likely to be compatible with user's existing shader.

    // Upload texture to graphics system
    GLint last_texture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glGenTextures(1, &g_FontTexture);
    glBindTexture(GL_TEXTURE_2D, g_FontTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    // Store our identifier
    io.Fonts->TexID = (void*)g_FontTexture;

    // Restore state
    glBindTexture(GL_TEXTURE_2D, last_texture);

    return true;
}

void ImGuiRenderPass::newFrame()
{
    ImGuiIO& io = ImGui::GetIO();

    // Setup display size (every frame to accommodate for window resizing)
    int w, h;
    int display_w, display_h;
    glfwGetWindowSize(g_Window, &w, &h);
    glfwGetFramebufferSize(g_Window, &display_w, &display_h);
    io.DisplaySize = ImVec2((float)w, (float)h);
    io.DisplayFramebufferScale = ImVec2((float)display_w / w, (float)display_h / h);

    io.DeltaTime = g_DeltaTime;

    io.KeyCtrl = glfwGetKey(g_Window, GLFW_KEY_LEFT_CONTROL) || glfwGetKey(g_Window, GLFW_KEY_RIGHT_CONTROL);
    io.KeyShift = glfwGetKey(g_Window, GLFW_KEY_LEFT_SHIFT) || glfwGetKey(g_Window, GLFW_KEY_RIGHT_SHIFT);
    io.KeyAlt = glfwGetKey(g_Window, GLFW_KEY_LEFT_ALT) || glfwGetKey(g_Window, GLFW_KEY_RIGHT_ALT);

    io.MouseWheel = g_MouseWheel;
    g_MouseWheel = 0;

    m_EventBroker->Process<ImGuiRenderPass>();

    ImGui::NewFrame();
}

