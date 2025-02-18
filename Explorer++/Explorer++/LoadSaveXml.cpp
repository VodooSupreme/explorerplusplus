// Copyright (C) Explorer++ Project
// SPDX-License-Identifier: GPL-3.0-only
// See LICENSE in the top level directory

#include "stdafx.h"
#include "LoadSaveXml.h"
// clang-format off
#include "Explorer++.h"
// clang-format on
#include "ApplicationModelFactory.h"
#include "ApplicationToolbarXmlStorage.h"
#include "Bookmarks/BookmarkTreeFactory.h"
#include "Bookmarks/BookmarkXmlStorage.h"
#include "ColorRuleModelFactory.h"
#include "ColorRuleXmlStorage.h"
#include "DialogHelper.h"
#include "Explorer++_internal.h"
#include "../Helper/ProcessHelper.h"
#include "../Helper/XMLSettings.h"
#include <wil/com.h>
#include <wil/resource.h>

LoadSaveXML::LoadSaveXML(Explorerplusplus *pContainer, BOOL bLoad) :
	m_pContainer(pContainer),
	m_bLoad(bLoad)
{
	if (bLoad)
	{
		/* Initialize the load environment (namely,
		load the configuration file). */
		InitializeLoadEnvironment();
	}
	else
	{
		/* Initialize the save environment. */
		InitializeSaveEnvironment();
	}
}

LoadSaveXML::~LoadSaveXML()
{
	if (!m_bLoad)
	{
		ReleaseSaveEnvironment();
	}
}

void LoadSaveXML::InitializeLoadEnvironment()
{
	m_pXMLDom.attach(NXMLSettings::DomFromCOM());

	if (!m_pXMLDom)
	{
		return;
	}

	TCHAR szConfigFile[MAX_PATH];
	GetProcessImageName(GetCurrentProcessId(), szConfigFile, SIZEOF_ARRAY(szConfigFile));
	PathRemoveFileSpec(szConfigFile);
	PathAppend(szConfigFile, NExplorerplusplus::XML_FILENAME);

	wil::unique_variant var(NXMLSettings::VariantString(NExplorerplusplus::XML_FILENAME));
	VARIANT_BOOL status;
	m_pXMLDom->load(var, &status);

	if (status != VARIANT_TRUE)
	{
		return;
	}
}

void LoadSaveXML::InitializeSaveEnvironment()
{
	m_pXMLDom.attach(NXMLSettings::DomFromCOM());

	if (!m_pXMLDom)
	{
		return;
	}

	/* Insert the XML header. */
	wil::com_ptr_nothrow<IXMLDOMProcessingInstruction> pi;
	auto bstr = wil::make_bstr_nothrow(L"xml");
	auto bstr1 = wil::make_bstr_nothrow(L"version='1.0'");
	m_pXMLDom->createProcessingInstruction(bstr.get(), bstr1.get(), &pi);
	NXMLSettings::AppendChildToParent(pi.get(), m_pXMLDom.get());

	/* Short header comment, explaining file purpose. */
	wil::com_ptr_nothrow<IXMLDOMComment> pc;
	bstr = wil::make_bstr_nothrow(L" Preference file for Explorer++ ");
	m_pXMLDom->createComment(bstr.get(), &pc);
	NXMLSettings::AppendChildToParent(pc.get(), m_pXMLDom.get());

	/* Create the root element. CANNOT use '+' signs
	within the element name. */
	bstr = wil::make_bstr_nothrow(L"ExplorerPlusPlus");
	m_pXMLDom->createElement(bstr.get(), &m_pRoot);

	NXMLSettings::AppendChildToParent(m_pRoot.get(), m_pXMLDom.get());

	auto bstr_wsnt = wil::make_bstr_nothrow(L"\n\t");
	NXMLSettings::AddWhiteSpaceToNode(m_pXMLDom.get(), bstr_wsnt.get(), m_pRoot.get());
}

void LoadSaveXML::ReleaseSaveEnvironment()
{
	auto bstr_wsn = wil::make_bstr_nothrow(L"\n");
	NXMLSettings::AddWhiteSpaceToNode(m_pXMLDom.get(), bstr_wsn.get(), m_pRoot.get());

	wil::unique_bstr bstr;
	m_pXMLDom->get_xml(&bstr);

	/* To ensure the configuration file is saved to the same directory
	as the executable, determine the fully qualified path of the executable,
	then save the configuration file in that directory. */
	TCHAR szConfigFile[MAX_PATH];
	DWORD dwProcessId = GetCurrentProcessId();
	wil::unique_process_handle process(
		OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessId));
	GetModuleFileNameEx(process.get(), nullptr, szConfigFile, SIZEOF_ARRAY(szConfigFile));

	PathRemoveFileSpec(szConfigFile);
	PathAppend(szConfigFile, NExplorerplusplus::XML_FILENAME);

	wil::unique_variant var(NXMLSettings::VariantString(szConfigFile));
	m_pXMLDom->save(var);
}

void LoadSaveXML::LoadGenericSettings()
{
	m_pContainer->LoadGenericSettingsFromXML(m_pXMLDom.get());
}

void LoadSaveXML::LoadBookmarks()
{
	BookmarkXmlStorage::Load(m_pXMLDom.get(),
		BookmarkTreeFactory::GetInstance()->GetBookmarkTree());
}

int LoadSaveXML::LoadPreviousTabs()
{
	return m_pContainer->LoadTabSettingsFromXML(m_pXMLDom.get());
}

void LoadSaveXML::LoadDefaultColumns()
{
	m_pContainer->LoadDefaultColumnsFromXML(m_pXMLDom.get());
}

void LoadSaveXML::LoadApplicationToolbar()
{
	Applications::ApplicationToolbarXmlStorage::Load(m_pXMLDom.get(),
		Applications::ApplicationModelFactory::GetInstance()->GetApplicationModel());
}

void LoadSaveXML::LoadToolbarInformation()
{
	m_pContainer->LoadToolbarInformationFromXML(m_pXMLDom.get());
}

void LoadSaveXML::LoadColorRules()
{
	ColorRuleXmlStorage::Load(m_pXMLDom.get(),
		ColorRuleModelFactory::GetInstance()->GetColorRuleModel());
}

void LoadSaveXML::LoadDialogStates()
{
	DialogHelper::LoadDialogStatesFromXML(m_pXMLDom.get());
}

void LoadSaveXML::SaveGenericSettings()
{
	m_pContainer->SaveGenericSettingsToXML(m_pXMLDom.get(), m_pRoot.get());
}

void LoadSaveXML::SaveBookmarks()
{
	BookmarkXmlStorage::Save(m_pXMLDom.get(), m_pRoot.get(),
		BookmarkTreeFactory::GetInstance()->GetBookmarkTree(), 1);
}

void LoadSaveXML::SaveTabs()
{
	m_pContainer->SaveTabSettingsToXML(m_pXMLDom.get(), m_pRoot.get());
}

void LoadSaveXML::SaveDefaultColumns()
{
	m_pContainer->SaveDefaultColumnsToXML(m_pXMLDom.get(), m_pRoot.get());
}

void LoadSaveXML::SaveApplicationToolbar()
{
	Applications::ApplicationToolbarXmlStorage::Save(m_pXMLDom.get(), m_pRoot.get(),
		Applications::ApplicationModelFactory::GetInstance()->GetApplicationModel());
}

void LoadSaveXML::SaveToolbarInformation()
{
	m_pContainer->SaveToolbarInformationToXML(m_pXMLDom.get(), m_pRoot.get());
}

void LoadSaveXML::SaveColorRules()
{
	ColorRuleXmlStorage::Save(m_pXMLDom.get(), m_pRoot.get(),
		ColorRuleModelFactory::GetInstance()->GetColorRuleModel());
}

void LoadSaveXML::SaveDialogStates()
{
	DialogHelper::SaveDialogStatesToXML(m_pXMLDom.get(), m_pRoot.get());
}
