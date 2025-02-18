// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#pragma once

#include "TabRestorer.h"
#include "../Helper/Macros.h"
#include <boost/signals2.hpp>
#include <wil/com.h>
#include <wil/resource.h>
#include <unordered_map>

class CoreInterface;

class TabRestorerUI
{
public:
	TabRestorerUI(HINSTANCE resourceInstance, CoreInterface *coreInterface,
		TabRestorer *tabRestorer, int menuStartId, int menuEndId);
	~TabRestorerUI();

	void OnMenuItemClicked(int menuItemId);

private:
	DISALLOW_COPY_AND_ASSIGN(TabRestorerUI);

	static const int MAX_MENU_ITEMS = 10;

	void OnMainMenuPreShow(HMENU mainMenu);
	wil::unique_hmenu BuildRecentlyClosedTabsMenu(std::vector<wil::unique_hbitmap> &menuImages,
		std::unordered_map<int, int> &menuItemMappings);

	HINSTANCE m_resourceInstance;
	CoreInterface *m_coreInterface;

	std::vector<boost::signals2::scoped_connection> m_connections;

	TabRestorer *m_tabRestorer;
	int m_menuStartId;
	int m_menuEndId;

	wil::unique_hmenu m_recentTabsMenu;
	std::vector<wil::unique_hbitmap> m_menuImages;
	wil::com_ptr_nothrow<IImageList> m_systemImageList;
	wil::unique_hbitmap m_defaultFolderIconBitmap;

	std::unordered_map<int, int> m_menuItemMappings;
};
