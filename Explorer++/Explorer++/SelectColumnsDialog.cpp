// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "stdafx.h"
#include "SelectColumnsDialog.h"
#include "DarkModeHelper.h"
#include "IconResourceLoader.h"
#include "MainResource.h"
#include "ResourceHelper.h"
#include "ShellBrowser/Columns.h"
#include "ShellBrowser/ShellBrowser.h"
#include "ShellBrowser/ShellNavigationController.h"
#include "../Helper/ListViewHelper.h"
#include "../Helper/Macros.h"
#include <algorithm>
#include <functional>

const TCHAR SelectColumnsDialogPersistentSettings::SETTINGS_KEY[] = _T("SelectColumns");

SelectColumnsDialog::SelectColumnsDialog(HINSTANCE resourceInstance, HWND hParent,
	ShellBrowser *shellBrowser, IconResourceLoader *iconResourceLoader) :
	DarkModeDialogBase(resourceInstance, IDD_SELECTCOLUMNS, hParent, true),
	m_shellBrowser(shellBrowser),
	m_iconResourceLoader(iconResourceLoader),
	m_bColumnsSwapped(FALSE)
{
	m_persistentSettings = &SelectColumnsDialogPersistentSettings::GetInstance();
}

INT_PTR SelectColumnsDialog::OnInitDialog()
{
	HWND hListView = GetDlgItem(m_hDlg, IDC_COLUMNS_LISTVIEW);
	SetWindowTheme(hListView, L"Explorer", nullptr);

	ListView_SetExtendedListViewStyleEx(hListView, LVS_EX_CHECKBOXES, LVS_EX_CHECKBOXES);

	LVCOLUMN lvColumn;
	lvColumn.mask = 0;
	ListView_InsertColumn(hListView, 0, &lvColumn);

	auto currentColumns = m_shellBrowser->GetCurrentColumns();

	std::sort(currentColumns.begin(), currentColumns.end(),
		std::bind_front(&SelectColumnsDialog::CompareColumns, this));

	int iItem = 0;

	for (const auto &column : currentColumns)
	{
		std::wstring text = ResourceHelper::LoadString(GetResourceInstance(),
			ShellBrowser::LookupColumnNameStringIndex(column.type));

		LVITEM lvItem;
		lvItem.mask = LVIF_TEXT | LVIF_PARAM;
		lvItem.iItem = iItem;
		lvItem.iSubItem = 0;
		lvItem.pszText = text.data();
		lvItem.lParam = static_cast<LPARAM>(column.type);
		ListView_InsertItem(hListView, &lvItem);

		ListView_SetCheckState(hListView, iItem, column.bChecked);

		iItem++;
	}

	ListView_SetColumnWidth(hListView, 0, LVSCW_AUTOSIZE);

	ListViewHelper::SelectItem(hListView, 0, TRUE);
	SetFocus(hListView);

	AllowDarkModeForControls({ IDC_COLUMNS_MOVEUP, IDC_COLUMNS_MOVEDOWN });

	auto &darkModeHelper = DarkModeHelper::GetInstance();

	if (darkModeHelper.IsDarkModeEnabled())
	{
		darkModeHelper.SetListViewDarkModeColors(hListView);
	}

	m_persistentSettings->RestoreDialogPosition(m_hDlg, true);

	return 0;
}

wil::unique_hicon SelectColumnsDialog::GetDialogIcon(int iconWidth, int iconHeight) const
{
	return m_iconResourceLoader->LoadIconFromPNGAndScale(Icon::SelectColumns, iconWidth,
		iconHeight);
}

bool SelectColumnsDialog::CompareColumns(const Column_t &column1, const Column_t &column2)
{
	if (column1.bChecked && column2.bChecked)
	{
		// If both column are checked, preserve the input ordering (this is
		// the order that the columns will actually appear in the listview).
		// This matches the behavior of Windows Explorer.
		return false;
	}
	else if (column1.bChecked && !column2.bChecked)
	{
		return true;
	}
	else if (!column1.bChecked && column2.bChecked)
	{
		return false;
	}

	TCHAR column1Text[64];
	LoadString(GetResourceInstance(), ShellBrowser::LookupColumnNameStringIndex(column1.type),
		column1Text, SIZEOF_ARRAY(column1Text));

	TCHAR column2Text[64];
	LoadString(GetResourceInstance(), ShellBrowser::LookupColumnNameStringIndex(column2.type),
		column2Text, SIZEOF_ARRAY(column2Text));

	int ret = StrCmpLogicalW(column1Text, column2Text);

	if (ret == -1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void SelectColumnsDialog::GetResizableControlInformation(BaseDialog::DialogSizeConstraint &dsc,
	std::list<ResizableDialog::Control> &ControlList)
{
	dsc = BaseDialog::DialogSizeConstraint::None;

	ResizableDialog::Control control;

	control.iID = IDC_COLUMNS_LISTVIEW;
	control.Type = ResizableDialog::ControlType::Resize;
	control.Constraint = ResizableDialog::ControlConstraint::None;
	ControlList.push_back(control);

	control.iID = IDC_COLUMNS_MOVEUP;
	control.Type = ResizableDialog::ControlType::Move;
	control.Constraint = ResizableDialog::ControlConstraint::X;
	ControlList.push_back(control);

	control.iID = IDC_COLUMNS_MOVEDOWN;
	control.Type = ResizableDialog::ControlType::Move;
	control.Constraint = ResizableDialog::ControlConstraint::X;
	ControlList.push_back(control);

	control.iID = IDC_STATIC_DESCRIPTION;
	control.Type = ResizableDialog::ControlType::Move;
	control.Constraint = ResizableDialog::ControlConstraint::Y;
	ControlList.push_back(control);

	control.iID = IDC_COLUMNS_DESCRIPTION;
	control.Type = ResizableDialog::ControlType::Move;
	control.Constraint = ResizableDialog::ControlConstraint::Y;
	ControlList.push_back(control);

	control.iID = IDC_COLUMNS_DESCRIPTION;
	control.Type = ResizableDialog::ControlType::Resize;
	control.Constraint = ResizableDialog::ControlConstraint::X;
	ControlList.push_back(control);

	control.iID = IDC_STATIC_ETCHEDHORZ;
	control.Type = ResizableDialog::ControlType::Move;
	control.Constraint = ResizableDialog::ControlConstraint::Y;
	ControlList.push_back(control);

	control.iID = IDC_STATIC_ETCHEDHORZ;
	control.Type = ResizableDialog::ControlType::Resize;
	control.Constraint = ResizableDialog::ControlConstraint::X;
	ControlList.push_back(control);

	control.iID = IDOK;
	control.Type = ResizableDialog::ControlType::Move;
	control.Constraint = ResizableDialog::ControlConstraint::None;
	ControlList.push_back(control);

	control.iID = IDCANCEL;
	control.Type = ResizableDialog::ControlType::Move;
	control.Constraint = ResizableDialog::ControlConstraint::None;
	ControlList.push_back(control);
}

INT_PTR SelectColumnsDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	switch (LOWORD(wParam))
	{
	case IDC_COLUMNS_MOVEUP:
		OnMoveColumn(true);
		break;

	case IDC_COLUMNS_MOVEDOWN:
		OnMoveColumn(false);
		break;

	case IDOK:
		OnOk();
		break;

	case IDCANCEL:
		OnCancel();
		break;
	}

	return 0;
}

INT_PTR SelectColumnsDialog::OnNotify(NMHDR *pnmhdr)
{
	switch (pnmhdr->code)
	{
	case LVN_ITEMCHANGING:
	{
		BOOL res = OnLvnItemChanging(reinterpret_cast<NMLISTVIEW *>(pnmhdr));
		SetWindowLongPtr(m_hDlg, DWLP_MSGRESULT, res);
		return TRUE;
	}

	case LVN_ITEMCHANGED:
		OnLvnItemChanged(reinterpret_cast<NMLISTVIEW *>(pnmhdr));
		break;
	}

	return 0;
}

INT_PTR SelectColumnsDialog::OnClose()
{
	EndDialog(m_hDlg, 0);
	return 0;
}

void SelectColumnsDialog::SaveState()
{
	m_persistentSettings->SaveDialogPosition(m_hDlg);
	m_persistentSettings->m_bStateSaved = TRUE;
}

void SelectColumnsDialog::OnOk()
{
	HWND hListView = GetDlgItem(m_hDlg, IDC_COLUMNS_LISTVIEW);
	std::vector<Column_t> updatedColumns;

	auto currentColumns = m_shellBrowser->GetCurrentColumns();

	for (int i = 0; i < ListView_GetItemCount(hListView); i++)
	{
		LVITEM lvItem;
		lvItem.mask = LVIF_PARAM;
		lvItem.iItem = i;
		lvItem.iSubItem = 0;
		ListView_GetItem(hListView, &lvItem);

		ColumnType columnType = static_cast<ColumnType>(lvItem.lParam);
		auto itr = std::find_if(currentColumns.begin(), currentColumns.end(),
			[columnType](const Column_t &column)
			{
				return column.type == columnType;
			});

		Column_t column;
		column.type = columnType;
		column.iWidth = itr->iWidth;
		column.bChecked = ListView_GetCheckState(hListView, i);
		updatedColumns.push_back(column);
	}

	m_shellBrowser->SetCurrentColumns(updatedColumns);

	if (m_bColumnsSwapped)
	{
		m_shellBrowser->GetNavigationController()->Refresh();
	}

	EndDialog(m_hDlg, 1);
}

void SelectColumnsDialog::OnCancel()
{
	EndDialog(m_hDlg, 0);
}

BOOL SelectColumnsDialog::OnLvnItemChanging(const NMLISTVIEW *nmlv)
{
	if (nmlv->uChanged != LVIF_STATE || (nmlv->uNewState & LVIS_STATEIMAGEMASK) == 0)
	{
		return FALSE;
	}

	BOOL checked = ((nmlv->uNewState & LVIS_STATEIMAGEMASK) >> 12) == 2;

	if (checked)
	{
		return FALSE;
	}

	HWND listView = GetDlgItem(m_hDlg, IDC_COLUMNS_LISTVIEW);
	int numItems = ListView_GetItemCount(listView);
	bool anyOtherItemsChecked = false;

	for (int i = 0; i < numItems; i++)
	{
		if (i == nmlv->iItem)
		{
			continue;
		}

		if (ListView_GetCheckState(listView, i))
		{
			anyOtherItemsChecked = true;
			break;
		}
	}

	if (!anyOtherItemsChecked)
	{
		// There should always be at least one column that's checked.
		return TRUE;
	}

	return FALSE;
}

void SelectColumnsDialog::OnLvnItemChanged(const NMLISTVIEW *pnmlv)
{
	if (pnmlv->uNewState & LVIS_SELECTED)
	{
		HWND hListView = GetDlgItem(m_hDlg, IDC_COLUMNS_LISTVIEW);

		LVITEM lvItem;
		lvItem.mask = LVIF_PARAM;
		lvItem.iItem = pnmlv->iItem;
		lvItem.iSubItem = 0;
		ListView_GetItem(hListView, &lvItem);

		int iDescriptionStringIndex = ShellBrowser::LookupColumnDescriptionStringIndex(
			static_cast<ColumnType>(lvItem.lParam));

		TCHAR szColumnDescription[128];
		LoadString(GetResourceInstance(), iDescriptionStringIndex, szColumnDescription,
			SIZEOF_ARRAY(szColumnDescription));
		SetDlgItemText(m_hDlg, IDC_COLUMNS_DESCRIPTION, szColumnDescription);
	}
}

void SelectColumnsDialog::OnMoveColumn(bool bUp)
{
	HWND hListView = GetDlgItem(m_hDlg, IDC_COLUMNS_LISTVIEW);

	int iSelected = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);

	if (iSelected != -1)
	{
		if (bUp)
		{
			ListViewHelper::SwapItems(hListView, iSelected, iSelected - 1, TRUE);
		}
		else
		{
			ListViewHelper::SwapItems(hListView, iSelected, iSelected + 1, TRUE);
		}

		m_bColumnsSwapped = TRUE;

		SetFocus(hListView);
	}
}

SelectColumnsDialogPersistentSettings::SelectColumnsDialogPersistentSettings() :
	DialogSettings(SETTINGS_KEY)
{
}

SelectColumnsDialogPersistentSettings &SelectColumnsDialogPersistentSettings::GetInstance()
{
	static SelectColumnsDialogPersistentSettings mfdps;
	return mfdps;
}
