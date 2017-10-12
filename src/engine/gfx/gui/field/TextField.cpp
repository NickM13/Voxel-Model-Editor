#include "engine\gfx\gui\field\TextField.h"


TextField::TextField(std::string p_compName, std::string p_title, Vector2<Sint32> p_pos, Vector2<Sint32> p_size, Sint8 p_colorTheme, bool p_limitField)
	: Component(p_compName, "", p_pos, p_size, ACTION)
{
	m_blankField = p_title;
	m_size = Vector2<Sint32>(p_size.x, Sint32(p_size.y));
	m_scrolling = false;
	m_scroll = {};
	m_cursorPos = {};
	m_selected = 0;
	splitTitle();
	m_limited = p_limitField;
	if(m_limited)
		m_limitSize = p_size;
}

void TextField::setScroll(Vector2<Sint32> p_scroll)
{
	m_scroll = p_scroll;
}
void TextField::setScrolling(bool p_state)
{
	m_scrolling = p_state;
}

void TextField::setTitle(std::string p_title)
{
	m_title = p_title;
	splitTitle();
	m_cursorPos = Vector2<Sint32>(Sint32(m_text[m_text.size() - 1].length()), Sint32(m_text.size() - 1));
}

void TextField::splitTitle()
{
	m_text.clear();
	Sint32 _i = 0;
	for(Sint32 i = 0; i < Sint32(m_title.length()) - 1; i++)
	{
		if(m_title[i] == '\n')
		{
			m_text.push_back(m_title.substr(_i, i - _i));
			_i = i + 1;
		}
	}
	m_text.push_back(m_title.substr(_i, m_title.length() - _i));
}

void TextField::mergeTitle()
{
	m_title = "";
	for(Uint16 i = 0; i < m_text.size(); i++)
	{
		m_title.append(m_text[i]);
		if(i < m_text.size() - 1)
			m_title += '\n';
	}
}

std::string TextField::getTitle()
{
	mergeTitle();
	return m_title;
}

void TextField::input(Sint8& p_interactFlags, Sint8* p_keyStates, Sint8* p_mouseStates, Vector2<Sint32> p_mousePos)
{
	p_mousePos = p_mousePos - m_pos;
	GLfloat _boxHeight = m_size.y * Font::getSpacingHeight();
	if((p_interactFlags & EVENT_MOUSEOVER) && p_mousePos.x >= 0 && p_mousePos.x < m_size.x
		&& p_mousePos.y >= 0 && p_mousePos.y < _boxHeight)
	{
		p_interactFlags -= EVENT_MOUSEOVER;
		addTooltip();
		if(p_mouseStates[GLFW_MOUSE_BUTTON_LEFT] & GMouse::MOUSE_PRESS)
			m_selected = 1;
	}
	else if(p_mouseStates[GLFW_MOUSE_BUTTON_LEFT] & GMouse::MOUSE_PRESS)
		m_selected = 0;
	if((p_interactFlags & EVENT_KEYPRESS) && m_selected != 0)
	{
		p_interactFlags -= EVENT_KEYPRESS;
		std::vector<GKey::keyPress> _keyEvents = GKey::m_keyEvents;
		for(Uint16 i = 0; i < _keyEvents.size(); i++)
		{
			if(_keyEvents[i].m_action != 0)
			{
				if(_keyEvents[i].m_keyCode == GLFW_KEY_ENTER)
				{
					if(m_cursorPos.y < m_limitSize.y - 1)
					{
						m_text.insert(m_text.begin() + m_cursorPos.y + 1, m_text[m_cursorPos.y].substr(m_cursorPos.x, m_text[m_cursorPos.y].length() - m_cursorPos.x));
						m_text[m_cursorPos.y].erase(m_cursorPos.x, m_text[m_cursorPos.y].length() - m_cursorPos.x);
						m_cursorPos.x = 0;
						m_cursorPos.y += 1;
					}
					else
					{
						m_selected = 0;
						callPressFunction();
					}
				}
				else if(_keyEvents[i].m_keyCode == GLFW_KEY_TAB)
				{
					m_selected = 0;
					callPressFunction();
				}
				else if(_keyEvents[i].m_keyCode == GLFW_KEY_ESCAPE)
				{
					m_selected = 0;
				}
				else if(_keyEvents[i].m_keyCode == GLFW_KEY_BACKSPACE)
				{
					if(m_cursorPos.x > 0 || m_cursorPos.y > 0)
					{
						if(m_cursorPos.x == 0)
						{
							m_cursorPos.x = m_text[m_cursorPos.y - 1].length();
							m_text[m_cursorPos.y - 1] = m_text[m_cursorPos.y - 1] + m_text[m_cursorPos.y];
							m_text.erase(m_text.begin() + m_cursorPos.y);
							m_cursorPos.y -= 1;
						}
						else
						{
							m_cursorPos.x -= 1;
							m_text[m_cursorPos.y].erase(m_text[m_cursorPos.y].begin() + m_cursorPos.x);
						}
					}
				}
				else if(_keyEvents[i].m_keyCode == GLFW_KEY_DELETE)
				{
					if(m_cursorPos.x < Sint32(m_text[m_text.size() - 1].length()) || m_cursorPos.y < Sint32(m_text.size()) - 1)
					{
						if(m_cursorPos.x == m_text[m_cursorPos.y].length())
						{
							m_text[m_cursorPos.y] = m_text[m_cursorPos.y] + m_text[m_cursorPos.y + 1];
							m_text.erase(m_text.begin() + m_cursorPos.y + 1);
						}
						else
							m_text[m_cursorPos.y].erase(m_text[m_cursorPos.y].begin() + m_cursorPos.x);
					}
				}
				else if((m_cursorPos.x < m_size.x && m_cursorPos.y < _boxHeight) || m_scrolling)
				{
					if(_keyEvents[i].m_keyCode >= 65 && _keyEvents[i].m_keyCode <= 90)
					{
						if((_keyEvents[i].m_mods & 1) == 0)
							m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, char(_keyEvents[i].m_keyCode + 32));
						else
							m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, char(_keyEvents[i].m_keyCode));
						m_cursorPos.x++;
					}
					else if(_keyEvents[i].m_keyCode == GLFW_KEY_SPACE)
					{
						m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, char(_keyEvents[i].m_keyCode));
						m_cursorPos.x++;
					}
					else if(_keyEvents[i].m_keyCode == GLFW_KEY_APOSTROPHE)
					{
						if((_keyEvents[i].m_mods & 1) == 0)
							m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, char(_keyEvents[i].m_keyCode));
						else
							m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, '\"');
						m_cursorPos.x++;
					}
					else if(_keyEvents[i].m_keyCode == GLFW_KEY_COMMA)
					{
						if((_keyEvents[i].m_mods & 1) == 0)
							m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, char(_keyEvents[i].m_keyCode));
						else
							m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, '<');
						m_cursorPos.x++;
					}
					else if(_keyEvents[i].m_keyCode == GLFW_KEY_PERIOD)
					{
						if((_keyEvents[i].m_mods & 1) == 0)
							m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, char(_keyEvents[i].m_keyCode));
						else
							m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, '>');
						m_cursorPos.x++;
					}
					else if(_keyEvents[i].m_keyCode == GLFW_KEY_SLASH)
					{
						if((_keyEvents[i].m_mods & 1) == 0)
							m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, char(_keyEvents[i].m_keyCode));
						else
							m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, '?');
						m_cursorPos.x++;
					}
					else if(_keyEvents[i].m_keyCode >= GLFW_KEY_0 && _keyEvents[i].m_keyCode <= GLFW_KEY_9)
					{
						if((_keyEvents[i].m_mods & 1) == 0)
						{
							m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, char(_keyEvents[i].m_keyCode));
						}
						else
						{
							switch(_keyEvents[i].m_keyCode)
							{
							case GLFW_KEY_1:
								m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, '!');
								break;
							case GLFW_KEY_2:
								m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, '@');
								break;
							case GLFW_KEY_3:
								m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, '#');
								break;
							case GLFW_KEY_4:
								m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, '$');
								break;
							case GLFW_KEY_5:
								m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, '%');
								break;
							case GLFW_KEY_6:
								m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, '^');
								break;
							case GLFW_KEY_7:
								m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, '&');
								break;
							case GLFW_KEY_8:
								m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, '*');
								break;
							case GLFW_KEY_9:
								m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, '(');
								break;
							case GLFW_KEY_0:
								m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, ')');
								break;
							default:
								break;
							}
						}
						m_cursorPos.x++;
					}
					else// if(m_scrolling)
					{
						if(_keyEvents[i].m_keyCode == GLFW_KEY_UP)
						{
							if(m_cursorPos.y > 0)
							{
								m_cursorPos.y -= 1;
								if(m_cursorPos.x > Sint32(m_text[m_cursorPos.y].length()))
									m_cursorPos.x = Sint32(m_text[m_cursorPos.y].length());
							}
							else if(m_cursorPos.x > 0)
								m_cursorPos.x = 0;
						}
						if(_keyEvents[i].m_keyCode == GLFW_KEY_RIGHT)
						{
							if(m_cursorPos.x < Sint32(m_text[m_cursorPos.y].length()))
								m_cursorPos.x += 1;
							else if(m_cursorPos.y < Sint32(m_text.size()) - 1)
							{
								m_cursorPos.x = 0;
								m_cursorPos.y += 1;
								if(m_cursorPos.x > Sint32(m_text[m_cursorPos.y].length()))
									m_cursorPos.x = Sint32(m_text[m_cursorPos.y].length());
							}
						}
						if(_keyEvents[i].m_keyCode == GLFW_KEY_DOWN)
						{
							if(m_cursorPos.y < Sint32(m_text.size()) - 1)
							{
								m_cursorPos.y += 1;
								if(m_cursorPos.x > Sint32(m_text[m_cursorPos.y].length()))
									m_cursorPos.x = Sint32(m_text[m_cursorPos.y].length());
							}
							else if(m_cursorPos.x < Sint32(m_text[m_cursorPos.y].length()))
								m_cursorPos.x = Sint32(m_text[m_cursorPos.y].length());
						}
						if(_keyEvents[i].m_keyCode == GLFW_KEY_LEFT)
						{
							if(m_cursorPos.x > 0)
								m_cursorPos.x -= 1;
							else if(m_cursorPos.y > 0)
							{
								m_cursorPos.y -= 1;
								m_cursorPos.x = Sint32(m_text[m_cursorPos.y].length());
							}
						}
					}
					if(m_cursorPos.x == m_size.x - 1 && m_cursorPos.y < _boxHeight - 1)
					{
						m_text.insert(m_text.begin() + m_cursorPos.y + 1, m_text[m_cursorPos.y].substr(m_cursorPos.x, m_text[m_cursorPos.y].length() - m_cursorPos.x));
						m_text[m_cursorPos.y].erase(m_cursorPos.x, m_text[m_cursorPos.y].length() - m_cursorPos.x);
						m_cursorPos.x = 0;
						m_cursorPos.y += 1;
					}
				}
			}
		}
	}

	if((p_interactFlags & EVENT_MOUSEOVER) &&
		p_mousePos.x >= 0 && p_mousePos.x < m_size.x
		&& p_mousePos.y >= 0 && p_mousePos.y < _boxHeight)
		p_interactFlags -= EVENT_MOUSEOVER;
}

void TextField::update(GLfloat p_deltaUpdate)
{

}

void TextField::render()
{
	GLfloat _boxHeight = m_size.y * Font::getSpacingHeight();
	glPushMatrix();
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		glTranslatef(GLfloat(m_pos.x), GLfloat(m_pos.y), 0);
		glBegin(GL_QUADS);
		{
			m_colorTheme.m_border.useColor();
			glVertex2f(-1, -1);
			glVertex2f(GLfloat(m_size.x + 1), -1);
			glVertex2f(GLfloat(m_size.x + 1), GLfloat(_boxHeight + 1));
			glVertex2f(-1, GLfloat(_boxHeight + 1));

			if(m_selected) m_colorTheme.m_select.useColor();
			else m_colorTheme.m_primary.useColor();
			glVertex2f(0, 0);
			glVertex2f(GLfloat(m_size.x), 0);
			glVertex2f(GLfloat(m_size.x), GLfloat(_boxHeight));
			glVertex2f(0, GLfloat(_boxHeight));
		}
		glEnd();
		m_colorTheme.m_text.useColor();
		Font::setAlignment(ALIGN_LEFT);
		if(m_title != "" || m_text[0] != "" || m_text.size() > 1)
		{
			for(Uint16 i = 0; i < m_text.size(); i++)
				Font::print(m_text[i], 2, Sint32((i + 0.5f) * Font::getSpacingHeight()));
			if(m_selected != 0 && (fmod(glfwGetTime(), 0.5) < 0.25))
				Font::print("|", Sint32(Font::getMessageWidth(m_text[m_cursorPos.y].substr(0, m_cursorPos.x)).x + 1), Sint32((m_cursorPos.y + 0.5f) * Font::getSpacingHeight()));
		}
		else
		{
			if(m_selected != 0)
			{
				m_colorTheme.m_text.useColor();
				Font::print(((fmod(glfwGetTime(), 0.5) < 0.25) ? "|" : ""), 2, Sint32(0.5f * Font::getSpacingHeight()));
			}
			m_colorTheme.m_text.useColor(1, 1, 1, 0.5f);
			for(Uint16 i = 0; i < fmin((_boxHeight), ceil(GLfloat(m_blankField.length()) / (m_size.x))); i++)
				Font::print(m_blankField.substr(i * (m_size.x), (m_size.x)) + ((m_selected && (i == (_boxHeight) - 1) && fmod(glfwGetTime(), 0.5) < 0.25) ? "|" : ""), 2, Sint32((i + 0.5f) * Font::getSpacingHeight()));
		}
	}
	glPopMatrix();
}
