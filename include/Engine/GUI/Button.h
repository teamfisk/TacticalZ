#ifndef GUI_BUTTON_H__
#define GUI_BUTTON_H__

#include "GUI/TextureFrame.h"
#include "GUI/EButtonEnter.h"
#include "GUI/EButtonLeave.h"
#include "GUI/EButtonPress.h"
#include "GUI/EButtonRelease.h"
#include "Core/EMouseMove.h"
#include "Core/EMousePress.h"
#include "Core/EMouseRelease.h"

namespace dd
{
namespace GUI
{

class Button : public TextureFrame
{
public:
	Button(Frame* parent, std::string name)
		: TextureFrame(parent, name)
	{
		EVENT_SUBSCRIBE_MEMBER(m_EMouseMove, &Button::OnMouseMove);
		EVENT_SUBSCRIBE_MEMBER(m_EMousePress, &Button::OnMousePress);
		EVENT_SUBSCRIBE_MEMBER(m_EMouseRelease, &Button::OnMouseRelease);
	}

	void SetTextureHover(std::string resourceName)
	{
		m_TextureHover = resourceName;
	}
	void SetTextureReleased(std::string resourceName)
	{
		m_TextureReleased = resourceName;
		SetTexture(resourceName);
	}
	void SetTexturePressed(std::string resourceName)
	{
		m_TexturePressed = resourceName;
	}

	void Draw(RenderScene& rq) override
	{
		if (m_Texture == nullptr && !m_TextureReleased.empty()) {
			SetTexture(m_TextureReleased);
		}

		TextureFrame::Draw(rq);
	}

	virtual void OnEnter() { }
	virtual void OnLeave() { }
	virtual void OnPress() { }
	virtual void OnRelease() { }

protected:
	bool m_MouseIsOver = false;
	bool m_IsDown = false;

	virtual bool OnMouseMove(const Events::MouseMove& event)
	{
		if (Hidden()) {
			return false;
		}

		bool isOver = Rectangle::Intersects(AbsoluteRectangle(), Rectangle(event.X, event.Y, 1, 1));
		if (isOver && !m_MouseIsOver) { // Enter
			if (!m_IsDown) {
				if (!m_TextureHover.empty()) {
					SetTexture(m_TextureHover);
				}
			}
			OnEnter();
			Events::ButtonEnter e;
			e.FrameName = m_Name;
			EventBroker->Publish(e);
			Events::PlaySound soundEvent;
			soundEvent.FilePath = "Sounds/GUI/hover-n.wav";
			EventBroker->Publish(soundEvent);
			
		} else if (!isOver && m_MouseIsOver) { // Leave
			if (!m_IsDown) {
				if (!m_TextureReleased.empty()) {
					SetTexture(m_TextureReleased);
				}
			}
			OnLeave();
			Events::ButtonLeave e;
			e.FrameName = m_Name;
			EventBroker->Publish(e);
		}
		m_MouseIsOver = isOver;

		return true;
	}
	virtual bool OnMousePress(const Events::MousePress& event)
	{
		if (Hidden()) {
			//LOG_DEBUG("Pressed hidden button");
			return false;
		}

		if (!Rectangle::Intersects(AbsoluteRectangle(), Rectangle(event.X, event.Y, 1, 1))) {
			return false;
		}

		if (!m_TexturePressed.empty()) {
			SetTexture(m_TexturePressed);
		}

		m_IsDown = true;
		OnPress();
		Events::ButtonPress e;
		e.FrameName = m_Name;
		e.Button = this;
		EventBroker->Publish(e);

		return true;
	}
	virtual bool OnMouseRelease(const Events::MouseRelease& event)
	{
		if (Hidden()) {
			//LOG_DEBUG("Released hidden button");
			return false;
		}

		bool isOver = Rectangle::Intersects(AbsoluteRectangle(), Rectangle(event.X, event.Y, 1, 1));
		if (!isOver && !m_IsDown) {
			return false;
		}

		if (m_MouseIsOver) {
			if (!m_TextureHover.empty()) {
				SetTexture(m_TextureHover);
			}
		} else {
			if (!m_TextureReleased.empty()) {
				SetTexture(m_TextureReleased);
			}
		}

		m_IsDown = false;
		OnRelease();
		Events::ButtonRelease e;
		e.FrameName = m_Name;
		e.Button = this;
		EventBroker->Publish(e);

		return true;
	}

private:
	EventRelay<Frame, Events::MouseMove> m_EMouseMove;
	EventRelay<Frame, Events::MousePress> m_EMousePress;
	EventRelay<Frame, Events::MouseRelease> m_EMouseRelease;

	std::string m_TextureHover;
	std::string m_TexturePressed;
	std::string m_TextureReleased;
};

}
}
#endif
