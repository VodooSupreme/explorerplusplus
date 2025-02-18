// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#pragma once

#include "Macros.h"
#include "MessageForwarder.h"
#include "ReferenceCount.h"
#include "ResizableDialog.h"
#include <wil/resource.h>
#include <functional>

/* Provides a degree of abstraction off a standard dialog.
For instance, provides the ability for a class to manage
a dialog without having to handle the dialog procedure
directly. */
class BaseDialog : public MessageForwarder
{
	friend INT_PTR CALLBACK BaseDialogProcStub(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
	enum class DialogSizeConstraint
	{
		None,
		X,
		Y
	};

	static const int RETURN_CANCEL = 0;
	static const int RETURN_OK = 1;

	virtual ~BaseDialog() = default;

	INT_PTR ShowModalDialog();
	HWND ShowModelessDialog(std::function<void()> dialogDestroyedObserver);

protected:
	BaseDialog(HINSTANCE resourceInstance, int iResource, HWND hParent, bool bResizable);

	virtual void OnInitDialogBase();
	virtual int GetGripperControlId() = 0;

	HINSTANCE GetResourceInstance() const;
	virtual wil::unique_hicon GetDialogIcon(int iconWidth, int iconHeight) const;

	INT_PTR GetDefaultReturnValue(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	HWND m_hDlg;
	int m_iMinWidth;
	int m_iMinHeight;

	HWND m_tipWnd;

private:
	DISALLOW_COPY_AND_ASSIGN(BaseDialog);

	INT_PTR CALLBACK BaseDialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual void GetResizableControlInformation(DialogSizeConstraint &dsc,
		std::list<ResizableDialog::Control> &controlList);
	virtual void SaveState();

	const HINSTANCE m_resourceInstance;
	const int m_iResource;
	const HWND m_hParent;
	std::function<void()> m_modelessDialogDestroyedObserver;

	wil::unique_hicon m_icon;

	bool m_showingModelessDialog = false;

	/* Used only with resizable dialogs. */
	const bool m_bResizable;
	DialogSizeConstraint m_dsc;
	std::unique_ptr<ResizableDialog> m_prd;
};
