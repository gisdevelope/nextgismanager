/******************************************************************************
 * Project:  wxGIS (GIS Catalog)
 * Purpose:  wxGxDBConnectionFactoryUI class.
 * Author:   Dmitry Baryshnikov (aka Bishop), polimax@mail.ru
 ******************************************************************************
*   Copyright (C) 2011,2013,2014 Dmitry Baryshnikov
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 2 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include "wxgis/catalogui/gxdbconnfactoryui.h"
#include "wxgis/catalogui/gxremoteconnui.h"
#include "wxgis/framework/icon.h"

#include "../../art/rdb_conn_16.xpm"
#include "../../art/rdb_conn_48.xpm"

//------------------------------------------------------------------------------
// wxGxDBConnectionFactoryUI
//------------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(wxGxDBConnectionFactoryUI, wxGxDBConnectionFactory)

wxGxDBConnectionFactoryUI::wxGxDBConnectionFactoryUI(void) : wxGxDBConnectionFactory()
{
    m_LargeIconConn = wxIcon(rdb_conn_48_xpm);
    m_SmallIconConn = wxIcon(rdb_conn_16_xpm);
    m_LargeIconDisconn = GetStateIcon(m_LargeIconConn, wxGISEnumIconStateDisconnect, true);
    m_SmallIconDisconn = GetStateIcon(m_SmallIconConn, wxGISEnumIconStateDisconnect, false);
}

wxGxDBConnectionFactoryUI::~wxGxDBConnectionFactoryUI(void)
{
}

wxGxObject* wxGxDBConnectionFactoryUI::GetGxObject(wxGxObject* pParent, const wxString &soName, const CPLString &szPath, bool bCheckNames)
{
    if(bCheckNames && IsNameExist(pParent, soName))
    {
        return NULL;
    }

    //TODO: other DB like MySQL my have different ifdefs
#ifdef wxGIS_USE_POSTGRES
	wxGxRemoteConnectionUI* pDataset = new wxGxRemoteConnectionUI(pParent, soName, szPath, m_LargeIconConn, m_SmallIconConn, m_LargeIconDisconn, m_SmallIconDisconn);
	return wxDynamicCast(pDataset, wxGxObject);
#else
    return NULL;
#endif //wxGIS_USE_POSTGRES
}
