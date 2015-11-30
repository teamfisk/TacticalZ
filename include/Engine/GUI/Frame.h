#ifndef GUI_Frame_h__
#define GUI_Frame_h__

#include "../Common.h"
#include "../Core/Util/Rectangle.h"
#include "../Core/EventBroker.h"
#include "../Core/EKeyDown.h"
#include "../Core/EKeyUp.h"
#include "../Core/ResourceManager.h"
#include "../Rendering/RenderQueue.h"
#include "../Rendering/Texture.h"
#include "../Input/EInputCommand.h"

namespace GUI
{

class Frame : public Rectangle
{
public:
	enum class Anchor
	{
		Left,
		Right,
		Top,
		Bottom
	};

	static const int BaseWidth = 1280;
	static const int BaseHeight = 720;

	// Set up a base frame with an event broker
	Frame(EventBroker* eventBroker)
		: m_EventBroker(eventBroker)
		, BaseFrame(this)
		, m_Name("UIParent")
		, Rectangle() { }

	// Create a frame as a child
	Frame(Frame* parent, std::string name)
		: m_Name(name)
	{
		SetParent(parent);
		Width = parent->Width;
		Height = parent->Height;
		EVENT_SUBSCRIBE_MEMBER(m_EKeyDown, &Frame::OnKeyDown);
		EVENT_SUBSCRIBE_MEMBER(m_EKeyUp, &Frame::OnKeyUp);
		EVENT_SUBSCRIBE_MEMBER(m_EInputCommand, &Frame::OnCommand);
	}

	~Frame()
	{
		/*for (auto layer : m_Children)
		{
			for (auto child : layer.second)
			{
				delete child.second;
			}
		}

		if (m_Parent)
		{
			m_Parent->RemoveChild(this);
		}*/
	}

	Frame* Parent() const { return m_Parent; }

	void SetParent(Frame* parent)
	{
		if (parent == nullptr) {
			LOG_ERROR("Failed to parent frame \"%s\": Invalid parent", m_Name.c_str());
			return;
		}

		m_Layer = parent->Layer() + 1;
		parent->AddChild(this);
		m_Parent = parent;
		m_EventBroker = parent->m_EventBroker;
		BaseFrame = parent->BaseFrame;
	}

	void AddChild(Frame* child)
	{
		m_Children[child->m_Layer].insert(std::make_pair(child->Name(), child));
		if (m_Parent) {
			m_Parent->AddChild(child);
		}
	}

	void RemoveChild(Frame* child)
	{
		auto it = m_Children.find(child->m_Layer);
		if (it != m_Children.end()) {
			m_Children.erase(it);
		}

		if (m_Parent) {
			m_Parent->RemoveChild(child);
		}
	}

	std::string Name() const { return m_Name; }
	void SetName(std::string val) { m_Name = val; }

	int Layer() const { return m_Layer; }

	bool Hidden() const
	{
		if (m_Parent)
			return m_Parent->Hidden() || m_Hidden;
		else
			return m_Hidden;
	}
	bool Visible() const
	{
		return !Hidden();
	}

	virtual void Hide() { m_Hidden = true; }
	virtual void Show() { m_Hidden = false; }

	int Left() const override
	{
		if (m_Parent)
			return m_Parent->Left() + X;
		else
			return X;
	}
	void SetLeft(int absLeft) override
	{
		if (m_Parent) {
			X = absLeft - m_Parent->Left();
		} else {
			X = absLeft;
		}
	}
	int Right() const override
	{
		return Left() + Width;
	}
	void SetRight(int absRight) override
	{
		if (m_Parent) {
			X = absRight - Width - m_Parent->Left();
		} else {
			X = absRight - Width;
		}
	}
	int Top() const override
	{
		if (m_Parent)
			return m_Parent->Top() + Y;
		else
			return Y;
	}
	void SetTop(int absTop) override
	{
		if (m_Parent) {
			Y = absTop - m_Parent->Top();
		} else {
			Y = absTop;
		}
	}
	int Bottom() const override
	{
		return Top() + Height;
	}
	void SetBottom(int absBottom) override
	{
		if (m_Parent) {
			Y = absBottom - Height - m_Parent->Top();
		} else {
			Y = absBottom - Height;
		}
	}

	glm::vec2 Scale()
	{
		if (m_Parent)
			return m_Parent->Scale();
		else
			return glm::vec2(Width, Height) / glm::vec2(BaseWidth, BaseHeight);
	}

	Rectangle AbsoluteRectangle()
	{
		int left = Left();
		if (m_Parent)
			left = std::max(left, m_Parent->Left());
		int top = Top();
		if (m_Parent)
			top = std::max(top, m_Parent->Top());
		int width = Right() - left;
		int height = Bottom() - top;
		return Rectangle(left, top, width, height);
	}

	void UpdateLayered(double dt)
	{
		// Update ourselves
		this->Update(dt);

		// Update children
		for (auto& pairLayer : m_Children) {
			auto children = pairLayer.second;
			for (auto& pairChild : children) {
				auto child = pairChild.second;
				child->Update(dt);
			}
		}
	}

	virtual void Update(double dt) { }

	void DrawLayered(RenderQueueCollection& rq)
	{
		if (this->Hidden())
			return;

		// Draw ourselves
		this->Draw(rq);

		// Draw children
		for (auto& pairLayer : m_Children) {
			auto children = pairLayer.second;
			for (auto& pairChild : children) {
				auto child = pairChild.second;
				if (child->Hidden())
					continue;
				child->Draw(rq);
			}
		}
	}

	virtual void Draw(RenderQueueCollection& rq) { }

protected:
	::EventBroker* m_EventBroker;
	Frame* BaseFrame = nullptr;

	std::string m_Name = "Unnamed";
	int m_Layer = 0;
	bool m_Hidden = false;

	Frame* m_Parent = nullptr;
	typedef std::multimap<std::string, Frame*> Children_t; // name -> frame
	std::map<int, Children_t> m_Children; // layer -> Children_t

	virtual bool OnKeyDown(const Events::KeyDown& event) { return false; }
	virtual bool OnKeyUp(const Events::KeyUp& event) { return false; }
	virtual bool OnCommand(const Events::InputCommand& event) { return false; }

private:
	EventRelay<Frame, Events::KeyDown> m_EKeyDown;
	EventRelay<Frame, Events::KeyUp> m_EKeyUp;
	EventRelay<Frame, Events::InputCommand> m_EInputCommand;
};

}

#endif
