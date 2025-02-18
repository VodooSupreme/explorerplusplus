// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "stdafx.h"
#include "Bookmarks/BookmarkIconManager.h"
#include "Bookmarks/BookmarkItem.h"
#include "CoreInterface.h"
#include "Icon.h"
#include "IconResourceLoader.h"
#include "../Helper/CachedIcons.h"
#include "../Helper/IconFetcher.h"
#include "../Helper/ImageHelper.h"

BookmarkIconManager::BookmarkIconManager(CoreInterface *coreInterface, IconFetcher *iconFetcher,
	int iconWidth, int iconHeight) :
	m_coreInterface(coreInterface),
	m_iconFetcher(iconFetcher),
	m_defaultFolderIconSystemImageListIndex(GetDefaultFolderIconIndex()),
	m_destroyed(std::make_shared<bool>(false))
{
	m_imageList.reset(ImageList_Create(iconWidth, iconHeight, ILC_COLOR32 | ILC_MASK, 0, 1));

	wil::unique_hbitmap folderIcon =
		m_coreInterface->GetIconResourceLoader()->LoadBitmapFromPNGAndScale(Icon::Folder, iconWidth,
			iconHeight);
	m_bookmarkFolderIconIndex = ImageList_Add(m_imageList.get(), folderIcon.get(), nullptr);

	SHGetImageList(SHIL_SYSSMALL, IID_PPV_ARGS(&m_systemImageList));
	m_defaultFolderIconIndex = ImageHelper::CopyImageListIcon(m_imageList.get(),
		reinterpret_cast<HIMAGELIST>(m_systemImageList.get()),
		m_defaultFolderIconSystemImageListIndex);
}

BookmarkIconManager::~BookmarkIconManager()
{
	*m_destroyed = true;
}

HIMAGELIST BookmarkIconManager::GetImageList()
{
	return m_imageList.get();
}

int BookmarkIconManager::GetBookmarkItemIconIndex(const BookmarkItem *bookmarkItem,
	IconAvailableCallback callback)
{
	int iconIndex;

	if (bookmarkItem->IsFolder())
	{
		iconIndex = m_bookmarkFolderIconIndex;
	}
	else
	{
		iconIndex = GetIconForBookmark(bookmarkItem, callback);
	}

	return iconIndex;
}

int BookmarkIconManager::GetIconForBookmark(const BookmarkItem *bookmark,
	IconAvailableCallback callback)
{
	int iconIndex = m_defaultFolderIconIndex;

	auto cachedItr = m_coreInterface->GetCachedIcons()->findByPath(bookmark->GetLocation());

	if (cachedItr != m_coreInterface->GetCachedIcons()->end())
	{
		iconIndex = AddSystemIconToImageList(cachedItr->iconIndex);
	}
	else
	{
		m_iconFetcher->QueueIconTask(bookmark->GetLocation(),
			[this, callback, destroyed = m_destroyed](int systemIconIndex)
			{
				if (*destroyed || !callback)
				{
					return;
				}

				if (systemIconIndex == m_defaultFolderIconSystemImageListIndex)
				{
					// Bookmarks use the standard folder icon by default, so if that's the icon
					// they're actually using, nothing else needs to happen.
					return;
				}

				int iconIndex = AddSystemIconToImageList(systemIconIndex);
				callback(iconIndex);
			});
	}

	return iconIndex;
}

int BookmarkIconManager::AddSystemIconToImageList(int systemIconIndex)
{
	// Note that while it would be possible to prevent other icons from being inserted into the
	// image list more than once, it would be a little more complicated (the number of uses for an
	// icon would have to be tracked) and there would be more room for things to go wrong.
	// It's likely not worth it, since most folders will probably use the default icon.
	if (systemIconIndex == m_defaultFolderIconSystemImageListIndex)
	{
		return m_defaultFolderIconIndex;
	}

	int iconIndex = ImageHelper::CopyImageListIcon(m_imageList.get(),
		reinterpret_cast<HIMAGELIST>(m_systemImageList.get()), systemIconIndex);

	return iconIndex;
}
