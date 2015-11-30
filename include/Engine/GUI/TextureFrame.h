#ifndef GUI_TextureFrame_h__
#define GUI_TextureFrame_h__

#include "Frame.h"
#include "../Rendering/Texture.h"

namespace GUI
{

class TextureFrame : public Frame
{
public:
	TextureFrame(Frame* parent, std::string name)
		: Frame(parent, name) { }

	void EnableScissor() { m_ScissorEnabled = true; }
	void DisableScissor() { m_ScissorEnabled = false; }

	void Draw(RenderQueueCollection& rq) override
	{
		if (m_Texture == nullptr)
			return;

		// Texture while fading
		if (m_FadeTexture && m_CurrentFade < 1) {
			FrameJob job;
			job.Scissor = (m_ScissorEnabled) ? m_Parent->AbsoluteRectangle() : Rectangle();
			job.Viewport = Rectangle(Left(), Top(), Width, Height);
			job.TextureID = m_FadeTexture->ResourceID;
			job.DiffuseTexture = m_FadeTexture;
			job.Color = glm::vec4(m_Color.r, m_Color.g, m_Color.b, m_Color.a);
			job.Name = Name();
			rq.GUI.Add(job);
		}

		// Main texture
		{
			FrameJob job;
			job.Scissor = (m_ScissorEnabled) ? m_Parent->AbsoluteRectangle() : Rectangle();
			job.Viewport = Rectangle(Left(), Top(), Width, Height);
			job.TextureID = m_Texture->ResourceID;
			job.DiffuseTexture = m_Texture;
			job.Color = glm::vec4(m_Color.r, m_Color.g, m_Color.b, m_Color.a * m_CurrentFade);
			job.Name = Name();
			rq.GUI.Add(job);
		}
	}

	std::string Texture() const { return m_TextureName; }

	void SetTexture(std::string resourceName)
	{
		if (resourceName.empty()) {
			m_Texture = nullptr;
			return;
		}

		m_Texture = ResourceManager::Load<Texture>(resourceName);
		m_TextureName = resourceName;
		if (m_Texture == nullptr) {
			m_Texture = ResourceManager::Load<Texture>("Textures/Core/ErrorTexture.png");
		}

		SizeToTexture();
	}

	void SizeToTexture()
	{
		if (m_Texture != nullptr) {
			this->Width = m_Texture->Width;
			this->Height = m_Texture->Height;
		}
	}

	void FadeToTexture(std::string resourceName, double duration)
	{
		m_FadeTexture = m_Texture;
		SetTexture(resourceName);
		m_FadeDuration = duration;
		m_CurrentFade = 0.f;
	}

	void Update(double dt) override
	{
		if (m_CurrentFade < 1) {
			m_CurrentFade += dt / m_FadeDuration;
			if (m_CurrentFade > 1) {
				m_FadeTexture = nullptr;
				m_CurrentFade = 1;
				m_FadeDuration = 0;
			}
		}
	}

	glm::vec4 Color() const { return m_Color; }
	void SetColor(glm::vec4 val) { m_Color = val; }

protected:
	bool m_ScissorEnabled = true;
	Texture* m_Texture = nullptr;
	std::string m_TextureName;
	Texture* m_FadeTexture = nullptr;
	glm::vec4 m_Color = glm::vec4(1.f, 1.f, 1.f, 1.f);
	float m_FadeDuration = 0.f;
	float m_CurrentFade = 1.f;

};

}

#endif
