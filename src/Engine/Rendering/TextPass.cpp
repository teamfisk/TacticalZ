#include "Rendering/TextPass.h"

TextPass::TextPass()
{

}

void TextPass::Initialize()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
	
    m_TextProgram = ResourceManager::Load<ShaderProgram>("#TextProgram");
    m_TextProgram->AddShader(std::shared_ptr<Shader>(new VertexShader("Shaders/Text.vert.glsl")));
    m_TextProgram->AddShader(std::shared_ptr<Shader>(new FragmentShader("Shaders/Text.frag.glsl")));
    m_TextProgram->Compile();
    m_TextProgram->BindFragDataLocation(0, "sceneColor");
    m_TextProgram->BindFragDataLocation(1, "bloomColor");
    m_TextProgram->Link();
}

void TextPass::Update()
{

}

void TextPass::Draw(RenderScene& scene, FrameBuffer& frameBuffer)
{
    GLERROR("Derp1");
    TextPassState* state = new TextPassState(frameBuffer.GetHandle());
    for (auto &job : scene.Jobs.Text) {
        auto textJob = std::dynamic_pointer_cast<TextJob>(job);
        if (textJob) {

            renderText(textJob->Content, textJob->Resource, textJob->Alignment, textJob->Color, textJob->Matrix, scene.Camera->ProjectionMatrix(), scene.Camera->ViewMatrix());
        }
    }
    GLERROR("Derp2");
    delete state;
}

std::string TextPass::parseColors(std::string text, std::map<int, glm::vec4>& colorChanges, glm::vec4 originalColor)
{
	std::string parsedString = text;
	glm::vec4 newColor = originalColor;
	bool colorChange = false;

	for (std::string::const_iterator c = parsedString.begin(); c != parsedString.end(); c++) {
		if (*c == char(92)) { // Backlash
			if (*(c + 1) == char('C')) { // C for Color 
				bool hasCorrectFormat = true;

				for (std::string::const_iterator colorC = c + 2; colorC != c + 8; colorC++) {
					if (*colorC < '0' || *colorC > 'F') {
						hasCorrectFormat = false;
						break;
					}
				}

				if (hasCorrectFormat == true) {
					colorChange = true;

					std::array<int, 3> hexToInt = {
						std::stoi(std::string(c + 2, c + 4), 0, 16),
						std::stoi(std::string(c + 4, c + 6), 0, 16),
						std::stoi(std::string(c + 6, c + 8), 0, 16)
					};

					newColor = glm::vec4(
						float(hexToInt[0]) / 255.f,
						float(hexToInt[1]) / 255.f,
						float(hexToInt[2]) / 255.f,
						newColor.a);

					parsedString.erase(c, (c + 8));
				}
			}

			if (*(c + 1) == char('A')) { // A for Alpha
				bool hasCorrectFormat = true;

				for (std::string::const_iterator colorC = c + 2; colorC != c + 4; colorC++) {
					if (*colorC < '0' || *colorC > 'F') {
						hasCorrectFormat = false;
						break;
					}

					if (hasCorrectFormat == true) {
						colorChange = true;

						int hexToInt = std::stoi(std::string(c + 2, c + 4), 0, 16);

						newColor = glm::vec4(
							newColor.r,
							newColor.g,
							newColor.b,
							float(hexToInt) / 255.f);

						parsedString.erase(c, (c + 4));
					}
				}
			}

			if (*(c + 1) > '0' && *(c + 1) < '9') { // Icons
				parsedString.replace(c, (c + 2), 1, (*(c + 1) - 48));
			}

			if (colorChange) {
				colorChanges[c - parsedString.begin()] = newColor;
			}
		}
	}



	return parsedString;
}

void TextPass::renderText(std::string text, Font* font, TextJob::AlignmentEnum alignment, glm::vec4 color, glm::mat4 modelMatrix, glm::mat4 projectionMatrix, glm::mat4 viewMatrix)
{
    GLfloat penX = 0;
    GLfloat penY = 0;
    GLfloat scale = 1.0/font->FontSize;

    GLfloat stringWidth = 0.f;

	std::map<int, glm::vec4> colorChanges;
	std::string parsedText = parseColors(text, colorChanges, color);

	for (std::string::const_iterator c = parsedText.begin(); c != parsedText.end(); c++) {
		Font::Character ch = font->m_Characters[*c];
		stringWidth += (ch.Advance >> 6) * scale;
	}

    if(alignment == TextJob::AlignmentEnum::Center) {
        penX = -stringWidth/2.f;
    } else if (alignment == TextJob::AlignmentEnum::Right) {
        penX = -stringWidth;
    } else {
        penX = 0;
    }	
    
    

    m_TextProgram->Bind();
    glUniform4fv(glGetUniformLocation(m_TextProgram->GetHandle(), "textColor"), 1, glm::value_ptr(color));
    glUniformMatrix4fv(glGetUniformLocation(m_TextProgram->GetHandle(), "M"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(m_TextProgram->GetHandle(), "V"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(m_TextProgram->GetHandle(), "P"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);


    for (std::string::const_iterator c = parsedText.begin(); c != parsedText.end(); c++) {

		auto it = colorChanges.find(c - parsedText.begin());
		if (it != colorChanges.end()) {
			glUniform4fv(glGetUniformLocation(m_TextProgram->GetHandle(), "textColor"), 1, glm::value_ptr(it->second));
		}

        Font::Character ch = font->m_Characters[*c];

        GLfloat xpos = penX + ch.Bearing.x * scale;
        GLfloat ypos = penY - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;

        GLfloat vertices[6][4] = {
            { xpos, ypos + h, 0.0, 0.0 },
            { xpos, ypos, 0.0, 1.0 },
            { xpos + w, ypos, 1.0, 1.0 },

            { xpos, ypos + h, 0.0, 0.0 },
            { xpos + w, ypos, 1.0, 1.0 },
            { xpos + w, ypos + h, 1.0, 0.0 }
        };

        glBindTexture(GL_TEXTURE_2D, ch.TextureID);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        penX += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    GLERROR("Text rendering Error");
}

