// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "stdafx.h"
#include "Explorer++.h"
#include "Icon.h"
#include "MainResource.h"
#include "ResourceHelper.h"
#include "../Helper/DpiCompatibility.h"
#include "../Helper/MenuHelper.h"
#include "../Helper/ShellHelper.h"
#include <wil/resource.h>
#include <map>

// clang-format off
const std::map<UINT, Icon> MAIN_MENU_IMAGE_MAPPINGS = {
	{ IDM_FILE_NEWTAB, Icon::NewTab},
	{ IDM_FILE_CLOSETAB, Icon::CloseTab },
	{ IDM_FILE_OPENCOMMANDPROMPT, Icon::CommandLine },
	{ IDM_FILE_OPENCOMMANDPROMPTADMINISTRATOR, Icon::CommandLineAdmin },
	{ IDM_FILE_DELETE, Icon::Delete },
	{ IDM_FILE_DELETEPERMANENTLY, Icon::DeletePermanently },
	{ IDM_FILE_RENAME, Icon::Rename },
	{ IDM_FILE_PROPERTIES, Icon::Properties },

	{ IDM_EDIT_UNDO, Icon::Undo },
	{ IDM_EDIT_COPY, Icon::Copy },
	{ IDM_EDIT_CUT, Icon::Cut },
	{ IDM_EDIT_PASTE, Icon::Paste },
	{ IDM_EDIT_PASTESHORTCUT, Icon::PasteShortcut },

	{ IDM_EDIT_COPYTOFOLDER, Icon::CopyTo },
	{ IDM_EDIT_MOVETOFOLDER, Icon::MoveTo },

	{ IDM_ACTIONS_NEWFOLDER, Icon::NewFolder },
	{ IDM_ACTIONS_SPLITFILE, Icon::SplitFiles },
	{ IDM_ACTIONS_MERGEFILES, Icon::MergeFiles },

	{ IDM_VIEW_REFRESH, Icon::Refresh },
	{ IDM_VIEW_SELECTCOLUMNS, Icon::SelectColumns },

	{ IDM_FILTER_FILTERRESULTS, Icon::Filter },

	{ IDM_GO_BACK, Icon::Back },
	{ IDM_GO_FORWARD, Icon::Forward },
	{ IDM_GO_UPONELEVEL, Icon::Up },

	{ IDM_TOOLS_SEARCH, Icon::Search },
	{ IDM_TOOLS_CUSTOMIZECOLORS, Icon::CustomizeColors },
	{ IDM_TOOLS_OPTIONS, Icon::Options },

	{ IDM_HELP_HELP, Icon::Help }
};
// clang-format on

void Explorerplusplus::InitializeMainMenu()
{
	// These need to occur after the language module has been initialized, but
	// before the tabs are restored.
	HMENU mainMenu = LoadMenu(m_resourceInstance, MAKEINTRESOURCE(IDR_MAINMENU));

	if (!m_commandLineSettings.enablePlugins)
	{
		DeleteMenu(mainMenu, IDM_TOOLS_RUNSCRIPT, MF_BYCOMMAND);
	}

	SetMenu(m_hContainer, mainMenu);

	AddViewModesToMenu(mainMenu, IDM_VIEW_PLACEHOLDER, FALSE);
	DeleteMenu(mainMenu, IDM_VIEW_PLACEHOLDER, MF_BYCOMMAND);

	SetMainMenuImages();

	InitializeGoMenu(mainMenu);
}

void Explorerplusplus::SetMainMenuImages()
{
	HMENU mainMenu = GetMenu(m_hContainer);
	UINT dpi = DpiCompatibility::GetInstance().GetDpiForWindow(m_hContainer);

	for (const auto &mapping : MAIN_MENU_IMAGE_MAPPINGS)
	{
		ResourceHelper::SetMenuItemImage(mainMenu, mapping.first, m_iconResourceLoader.get(),
			mapping.second, dpi, m_menuImages);
	}
}

void Explorerplusplus::InitializeGoMenu(HMENU mainMenu)
{
	// This is a bit indirect, but it's better than using something like GetSubMenu(), which would
	// rely on the "Go" menu remaining in a fixed position.
	HMENU goMenu = MenuHelper::FindParentMenu(mainMenu, IDM_GO_BACK);
	assert(goMenu);

	MenuHelper::AddSeparator(goMenu);

	// This is the quick access/home folder in Windows 10/11.
	AddGoMenuItem(goMenu, IDM_GO_QUICK_ACCESS, QUICK_ACCESS_PATH);
	AddGoMenuItem(goMenu, IDM_GO_COMPUTER, FOLDERID_ComputerFolder);

	MenuHelper::AddSeparator(goMenu);

	AddGoMenuItem(goMenu, IDM_GO_DOCUMENTS, FOLDERID_Documents);
	AddGoMenuItem(goMenu, IDM_GO_DOWNLOADS, FOLDERID_Downloads);
	AddGoMenuItem(goMenu, IDM_GO_MUSIC, FOLDERID_Music);
	AddGoMenuItem(goMenu, IDM_GO_PICTURES, FOLDERID_Pictures);
	AddGoMenuItem(goMenu, IDM_GO_VIDEOS, FOLDERID_Videos);
	AddGoMenuItem(goMenu, IDM_GO_DESKTOP, FOLDERID_Desktop);

	MenuHelper::AddSeparator(goMenu);

	AddGoMenuItem(goMenu, IDM_GO_RECYCLE_BIN, FOLDERID_RecycleBinFolder);
	AddGoMenuItem(goMenu, IDM_GO_CONTROL_PANEL, FOLDERID_ControlPanelFolder);
	AddGoMenuItem(goMenu, IDM_GO_PRINTERS, FOLDERID_PrintersFolder);
	AddGoMenuItem(goMenu, IDM_GO_NETWORK, FOLDERID_NetworkFolder);

	MenuHelper::AddSeparator(goMenu);

	AddGoMenuItem(goMenu, IDM_GO_WSL_DISTRIBUTIONS, WSL_DISTRIBUTIONS_PATH);

	MenuHelper::RemoveTrailingSeparators(goMenu);
}

void Explorerplusplus::AddGoMenuItem(HMENU goMenu, UINT id, const KNOWNFOLDERID &folderId)
{
	unique_pidl_absolute pidl;
	HRESULT hr = SHGetKnownFolderIDList(folderId, KF_FLAG_DEFAULT, nullptr, wil::out_param(pidl));

	if (FAILED(hr))
	{
		return;
	}

	AddGoMenuItem(goMenu, id, pidl.get());
}

void Explorerplusplus::AddGoMenuItem(HMENU goMenu, UINT id, const std::wstring &path)
{
	unique_pidl_absolute pidl;
	HRESULT hr = SHParseDisplayName(path.c_str(), nullptr, wil::out_param(pidl), 0, nullptr);

	if (FAILED(hr))
	{
		return;
	}

	AddGoMenuItem(goMenu, id, pidl.get());
}

void Explorerplusplus::AddGoMenuItem(HMENU goMenu, UINT id, PCIDLIST_ABSOLUTE pidl)
{
	std::wstring folderName;
	HRESULT hr = GetDisplayName(pidl, SHGDN_INFOLDER, folderName);

	if (FAILED(hr))
	{
		return;
	}

	MenuHelper::AddStringItem(goMenu, id, folderName);
}

boost::signals2::connection Explorerplusplus::AddMainMenuPreShowObserver(
	const MainMenuPreShowSignal::slot_type &observer)
{
	return m_mainMenuPreShowSignal.connect(observer);
}
