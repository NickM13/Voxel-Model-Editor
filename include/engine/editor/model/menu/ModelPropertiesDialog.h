#pragma once

#include "engine\gfx\gui\LGui.h"

class ModelPropertiesDialog : public Singleton<ModelPropertiesDialog> {
private:
	CDialog* m_dialog;

public:
	ModelPropertiesDialog();
	CDialog* getDialog();
};