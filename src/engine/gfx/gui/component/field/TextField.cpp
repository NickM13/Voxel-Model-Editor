#include "engine\gfx\gui\component\field\TextField.h"

TextField::TextField(std::string p_compName, std::string p_title, Vector2<Sint32> p_pos, Vector2<Sint32> p_size, Sint8 p_colorTheme, bool p_limitField)
	: Component(p_compName, "", p_pos, p_size) {
	m_blankField = p_title;
	m_size = p_size;
	m_scrolling = false;
	m_scroll = {};
	m_cursorPos = {};
	m_selected = 0;
	splitTitle();
	m_limited = p_limitField;
	if (m_limited) {
		m_limitSize = p_size;
	}
}

void TextField::setScroll(Vector2<Sint32> p_scroll) {
	m_scroll = p_scroll;
}
void TextField::setScrolling(bool p_state) {
	m_scrolling = p_state;
}

void TextField::setTitle(std::string p_title) {
	m_title = p_title;
	splitTitle();
	m_cursorPos = Vector2<Sint32>(Sint32(m_text[m_text.size() - 1].length()), Sint32(m_text.size() - 1));
}

void TextField::splitTitle() {
	m_text.clear();
	Sint32 _i = 0;
	for (Sint32 i = 0; i < Sint32(m_title.length()) - 1; i++) {
		if (m_title[i] == '\n') {
			m_text.push_back(m_title.substr(_i, i - _i));
			_i = i + 1;
		}
	}
	m_text.push_back(m_title.substr(_i, m_title.length() - _i));
}

void TextField::mergeTitle() {
	m_title = "";
	for (Uint16 i = 0; i < m_text.size(); i++) {
		m_title.append(m_text[i]);
		if (i < m_text.size() - 1)
			m_title += '\n';
	}
}

std::string TextField::getTitle() {
	mergeTitle();
	return m_title;
}

void TextField::input(Sint8& p_interactFlags) {
	Vector2<Sint32> _mousePos = GMouse::getMousePos() - m_pos;
	Sint32 _boxHeight = static_cast<Sint32>(m_size.y * Font::getSpacingHeight());
	if ((p_interactFlags & (Sint8)EventFlag::MOUSEOVER) && _mousePos.x >= 0 && _mousePos.x < m_size.x
		&& _mousePos.y >= 0 && _mousePos.y < _boxHeight) {
		p_interactFlags -= (Sint8)EventFlag::MOUSEOVER;
		addTooltip();
		if (GMouse::mousePressed(GLFW_MOUSE_BUTTON_LEFT)) {
			Sint32 row = _mousePos.y / Font::getSpacingHeight();
			if (row >= (Sint32)m_text.size()) row = (Sint32)m_text.size() - 1;
			Sint32 col = 0;
			Sint32 currLength = 0;
			m_cursorPos.x = m_text[row].length();
			for (Sint32 i = 0; i < (Sint32)m_text[row].length(); i++) {
				currLength += Font::getCharSize(m_text[row][i]).x;
				if (_mousePos.x < currLength) {
					m_cursorPos.x = i;
					break;
				}
			}
			m_selected = 1;
		}
		GGui::setCursorType(GGui::CursorType::IBEAM);
	} else if (GMouse::mousePressed(GLFW_MOUSE_BUTTON_LEFT))
		m_selected = 0;
	if ((p_interactFlags & (Sint8)EventFlag::KEYPRESS) && m_selected != 0) {
		p_interactFlags -= (Sint8)EventFlag::KEYPRESS;
		std::vector<GKey::KeyEvent> _keyEvents = GKey::getKeyEvents();
		for (Uint16 i = 0; i < _keyEvents.size(); i++) {
			if (_keyEvents[i].action != 0) {
				if (_keyEvents[i].keyCode == GLFW_KEY_ENTER || _keyEvents[i].keyCode == 335) { // 335 is numpad enter
					if (!m_limited || m_cursorPos.y < m_limitSize.y - 1) {
						m_text.insert(m_text.begin() + m_cursorPos.y + 1, m_text[m_cursorPos.y].substr(m_cursorPos.x, m_text[m_cursorPos.y].length() - m_cursorPos.x));
						m_text[m_cursorPos.y].erase(m_cursorPos.x, m_text[m_cursorPos.y].length() - m_cursorPos.x);
						m_cursorPos.x = 0;
						m_cursorPos.y += 1;
					} else {
						nextComponent();
					}
				} else if (_keyEvents[i].keyCode == GLFW_KEY_TAB) {
					if (GKey::modDown(GLFW_MOD_SHIFT))
						prevComponent();
					else
						nextComponent();
				} else if (_keyEvents[i].keyCode == GLFW_KEY_ESCAPE) {
					m_selected = 0;
				} else if (_keyEvents[i].keyCode == GLFW_KEY_BACKSPACE) {
					if (m_cursorPos.x > 0 || m_cursorPos.y > 0) {
						if (m_cursorPos.x == 0) {
							m_cursorPos.x = m_text[m_cursorPos.y - 1].length();
							m_text[m_cursorPos.y - 1] = m_text[m_cursorPos.y - 1] + m_text[m_cursorPos.y];
							m_text.erase(m_text.begin() + m_cursorPos.y);
							m_cursorPos.y -= 1;
						} else {
							m_cursorPos.x -= 1;
							m_text[m_cursorPos.y].erase(m_text[m_cursorPos.y].begin() + m_cursorPos.x);
						}
					}
				} else if (_keyEvents[i].keyCode == GLFW_KEY_DELETE) {
					if (m_cursorPos.x < Sint32(m_text[m_text.size() - 1].length()) || m_cursorPos.y < Sint32(m_text.size()) - 1) {
						if (m_cursorPos.x == m_text[m_cursorPos.y].length()) {
							m_text[m_cursorPos.y] = m_text[m_cursorPos.y] + m_text[m_cursorPos.y + 1];
							m_text.erase(m_text.begin() + m_cursorPos.y + 1);
						} else
							m_text[m_cursorPos.y].erase(m_text[m_cursorPos.y].begin() + m_cursorPos.x);
					}
				} else if ((m_cursorPos.x < m_size.x && m_cursorPos.y < _boxHeight) || m_scrolling) {
					if (_keyEvents[i].keyCode >= 65 && _keyEvents[i].keyCode <= 90) {
						if (!GKey::modDown(GLFW_MOD_SHIFT))
							m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, char(_keyEvents[i].keyCode + 32));
						else
							m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, char(_keyEvents[i].keyCode));
						m_cursorPos.x++;
					} else if (_keyEvents[i].keyCode == GLFW_KEY_SPACE) {
						m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, char(_keyEvents[i].keyCode));
						m_cursorPos.x++;
					} else if (_keyEvents[i].keyCode == GLFW_KEY_APOSTROPHE) {
						if (!GKey::modDown(GLFW_MOD_SHIFT))
							m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, char(_keyEvents[i].keyCode));
						else
							m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, '\"');
						m_cursorPos.x++;
					} else if (_keyEvents[i].keyCode == GLFW_KEY_COMMA) {
						if (!GKey::modDown(GLFW_MOD_SHIFT))
							m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, char(_keyEvents[i].keyCode));
						else
							m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, '<');
						m_cursorPos.x++;
					} else if (_keyEvents[i].keyCode == GLFW_KEY_PERIOD) {
						if (!GKey::modDown(GLFW_MOD_SHIFT))
							m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, char(_keyEvents[i].keyCode));
						else
							m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, '>');
						m_cursorPos.x++;
					} else if (_keyEvents[i].keyCode == GLFW_KEY_SLASH) {
						if (!GKey::modDown(GLFW_MOD_SHIFT))
							m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, char(_keyEvents[i].keyCode));
						else
							m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, '?');
						m_cursorPos.x++;
					} else if (_keyEvents[i].keyCode == GLFW_KEY_EQUAL) {
						if (!GKey::modDown(GLFW_MOD_SHIFT))
							m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, char(_keyEvents[i].keyCode));
						else
							m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, '+');
						m_cursorPos.x++;
					} else if (_keyEvents[i].keyCode == GLFW_KEY_MINUS) {
						if (!GKey::modDown(GLFW_MOD_SHIFT))
							m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, char(_keyEvents[i].keyCode));
						else
							m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, '_');
						m_cursorPos.x++;
					} else if (_keyEvents[i].keyCode == GLFW_KEY_LEFT_BRACKET) {
						if (!GKey::modDown(GLFW_MOD_SHIFT))
							m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, char(_keyEvents[i].keyCode));
						else
							m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, '{');
						m_cursorPos.x++;
					} else if (_keyEvents[i].keyCode == GLFW_KEY_RIGHT_BRACKET) {
						if (!GKey::modDown(GLFW_MOD_SHIFT))
							m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, char(_keyEvents[i].keyCode));
						else
							m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, '}');
						m_cursorPos.x++;
					} else if (_keyEvents[i].keyCode == '\\') {
						if (!GKey::modDown(GLFW_MOD_SHIFT))
							m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, char(_keyEvents[i].keyCode));
						else
							m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, '|');
						m_cursorPos.x++;
					} else if (_keyEvents[i].keyCode == GLFW_KEY_SEMICOLON) {
						if (!GKey::modDown(GLFW_MOD_SHIFT))
							m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, char(_keyEvents[i].keyCode));
						else
							m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, ':');
						m_cursorPos.x++;
					} else if (_keyEvents[i].keyCode == '`') {
						if (!GKey::modDown(GLFW_MOD_SHIFT))
							m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, char(_keyEvents[i].keyCode));
						else
							m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, '~');
						m_cursorPos.x++;
					} else if (_keyEvents[i].keyCode >= GLFW_KEY_0 && _keyEvents[i].keyCode <= GLFW_KEY_9) {
						if (!GKey::modDown(GLFW_MOD_SHIFT)) {
							m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, char(_keyEvents[i].keyCode));
						} else {
							switch (_keyEvents[i].keyCode) {
							case GLFW_KEY_1: m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, '!'); break;
							case GLFW_KEY_2: m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, '@'); break;
							case GLFW_KEY_3: m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, '#'); break;
							case GLFW_KEY_4: m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, '$'); break;
							case GLFW_KEY_5: m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, '%'); break;
							case GLFW_KEY_6: m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, '^'); break;
							case GLFW_KEY_7: m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, '&'); break;
							case GLFW_KEY_8: m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, '*'); break;
							case GLFW_KEY_9: m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, '('); break;
							case GLFW_KEY_0: m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, ')'); break;
							default: break;
							}
						}
						m_cursorPos.x++;
					} else if (_keyEvents[i].keyCode >= 320 && _keyEvents[i].keyCode <= 329) { // NUMPAD 0-9
						m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, char(_keyEvents[i].keyCode - 320 + GLFW_KEY_0));
						m_cursorPos.x++;
					} else if (_keyEvents[i].keyCode == 330) { // NUMPAD .
						m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, '.');
						m_cursorPos.x++;
					} else if (_keyEvents[i].keyCode == 331) { // NUMPAD /
						m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, '/');
						m_cursorPos.x++;
					} else if (_keyEvents[i].keyCode == 332) { // NUMPAD *
						m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, '*');
						m_cursorPos.x++;
					} else if (_keyEvents[i].keyCode == 333) { // NUMPAD -
						m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, '-');
						m_cursorPos.x++;
					} else if (_keyEvents[i].keyCode == 334) { // NUMPAD +
						m_text[m_cursorPos.y].insert(m_text[m_cursorPos.y].begin() + m_cursorPos.x, '+');
						m_cursorPos.x++;
					} else {
						if (_keyEvents[i].keyCode == GLFW_KEY_UP) {
							if (m_cursorPos.y > 0) {
								m_cursorPos.y -= 1;
								if (m_cursorPos.x > Sint32(m_text[m_cursorPos.y].length()))
									m_cursorPos.x = Sint32(m_text[m_cursorPos.y].length());
							} else if (m_cursorPos.x > 0)
								m_cursorPos.x = 0;
						}
						if (_keyEvents[i].keyCode == GLFW_KEY_RIGHT) {
							if (m_cursorPos.x < Sint32(m_text[m_cursorPos.y].length()))
								m_cursorPos.x += 1;
							else if (m_cursorPos.y < Sint32(m_text.size()) - 1) {
								m_cursorPos.x = 0;
								m_cursorPos.y += 1;
								if (m_cursorPos.x > Sint32(m_text[m_cursorPos.y].length()))
									m_cursorPos.x = Sint32(m_text[m_cursorPos.y].length());
							}
						}
						if (_keyEvents[i].keyCode == GLFW_KEY_DOWN) {
							if (m_cursorPos.y < Sint32(m_text.size()) - 1) {
								m_cursorPos.y += 1;
								if (m_cursorPos.x > Sint32(m_text[m_cursorPos.y].length()))
									m_cursorPos.x = Sint32(m_text[m_cursorPos.y].length());
							} else if (m_cursorPos.x < Sint32(m_text[m_cursorPos.y].length()))
								m_cursorPos.x = Sint32(m_text[m_cursorPos.y].length());
						}
						if (_keyEvents[i].keyCode == GLFW_KEY_LEFT) {
							if (m_cursorPos.x > 0)
								m_cursorPos.x -= 1;
							else if (m_cursorPos.y > 0) {
								m_cursorPos.y -= 1;
								m_cursorPos.x = Sint32(m_text[m_cursorPos.y].length());
							}
						}
					}
					if (m_cursorPos.x == m_size.x - 1 && m_cursorPos.y < _boxHeight - 1) {
						m_text.insert(m_text.begin() + m_cursorPos.y + 1, m_text[m_cursorPos.y].substr(m_cursorPos.x, m_text[m_cursorPos.y].length() - m_cursorPos.x));
						m_text[m_cursorPos.y].erase(m_cursorPos.x, m_text[m_cursorPos.y].length() - m_cursorPos.x);
						m_cursorPos.x = 0;
						m_cursorPos.y += 1;
					}
				}
			}
		}
	}

	if ((p_interactFlags & (Sint8)EventFlag::MOUSEOVER) &&
		_mousePos.x >= 0 && _mousePos.x < m_size.x
		&& _mousePos.y >= 0 && _mousePos.y < _boxHeight)
		p_interactFlags -= (Sint8)EventFlag::MOUSEOVER;
}

void TextField::update(GLfloat p_deltaUpdate) {

}

void TextField::render() {
	Sint32 _boxHeight = static_cast<Sint32>(m_size.y * Font::getSpacingHeight());
	Shader::pushMatrixModel();
	GBuffer::setTexture(0);
	Shader::translate(glm::vec3((GLfloat)m_pos.x, (GLfloat)m_pos.y, 0.f));

	GBuffer::renderShadow({}, { m_size.x, _boxHeight });
	GBuffer::setTexture(0);

	GBuffer::setColor(getElementColor(getElementPos() + "ActionPressed"));

	GBuffer::addVertexQuad(0, 0);
	GBuffer::addVertexQuad(GLfloat(m_size.x), 0);
	GBuffer::addVertexQuad(GLfloat(m_size.x), GLfloat(_boxHeight));
	GBuffer::addVertexQuad(0, GLfloat(_boxHeight));

	GBuffer::setColor(getElementColor(getElementPos() + "Text1"));
	Font::setAlignment(ALIGN_LEFT);
	if (m_title != "" || m_text[0] != "" || m_text.size() > 1) {
		for (Uint16 i = 0; i < m_text.size(); i++) {
			Font::print(m_text[i], 2, Sint32((i + 0.5f) * Font::getSpacingHeight()));
		}
		if (m_selected != 0 && (fmod(glfwGetTime(), 0.5) < 0.25)) {
			Font::print("|",
				Sint32(Font::getMessageWidth(m_text[m_cursorPos.y].substr(0, m_cursorPos.x)).x + 1) + 0,
				Sint32((m_cursorPos.y + 0.5f) * Font::getSpacingHeight() - 2));
		}
	} else {
		if (m_selected != 0) {
			GBuffer::setColor(getElementColor(getElementPos() + "Text1"));
			Font::print(((fmod(glfwGetTime(), 0.5) < 0.25) ? "|" : ""),
				2,
				Sint32(0.5f * Font::getSpacingHeight() - 2));
		}
		GBuffer::setColor(getElementColor(getElementPos() + "Text1").applyScale(1.f, 1.f, 1.f, 0.5f));
		for (Uint16 i = 0; i < fmin((_boxHeight), ceil(GLfloat(m_blankField.length()) / (m_size.x))); i++) {
			Font::print(m_blankField.substr(i * (m_size.x), (m_size.x)) + ((m_selected && (i == (_boxHeight)-1) && fmod(glfwGetTime(), 0.5) < 0.25) ? "|" : ""),
				2,
				Sint32((i + 0.5f) * Font::getSpacingHeight()));
		}
	}
	Shader::popMatrixModel();
}
