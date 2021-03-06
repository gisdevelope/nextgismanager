/******************************************************************************
 * Project:  wxGIS (GIS Catalog)
 * Purpose:  wxGISVectorPropertyPage class.
 * Author:   Dmitry Baryshnikov (aka Bishop), polimax@mail.ru
 ******************************************************************************
*   Copyright (C) 2010-2014 Dmitry Baryshnikov
*   Copyright (C) 2014 NextGIS
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
#include "wxgis/catalogui/vectorpropertypage.h"
#include "wxgis/datasource/sysop.h"

#include "../../art/shp_dset_16.xpm"

#include <wx/propgrid/advprops.h>
#include <wx/fontmap.h>

//--------------------------------------------------------------------------
// wxGISVectorPropertyPage
//--------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(wxGISVectorPropertyPage, wxGxPropertyPage)

BEGIN_EVENT_TABLE(wxGISVectorPropertyPage, wxGxPropertyPage)
	EVT_CHILD_FOCUS( wxGISVectorPropertyPage::OnChildFocus )
END_EVENT_TABLE()

wxGISVectorPropertyPage::wxGISVectorPropertyPage(void) : wxGxPropertyPage()
{
    m_pDataset = NULL;
    m_pGxDataset = NULL;
    m_pg = NULL;
	m_nCounter = 0;	
	m_sPageName = wxString(_("Vector"));	
	m_PageIcon = wxBitmap(shp_dset_16_xpm);
}

wxGISVectorPropertyPage::wxGISVectorPropertyPage(ITrackCancel * const pTrackCancel, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name) : wxGxPropertyPage()
{
    m_pDataset = NULL;
    m_pGxDataset = NULL;
    m_pg = NULL;
	m_nCounter = 0;	
	m_sPageName = wxString(_("Vector"));	
	m_PageIcon = wxBitmap(shp_dset_16_xpm);
    Create(pTrackCancel, parent, id, pos, size, style, name);
}

wxGISVectorPropertyPage::~wxGISVectorPropertyPage()
{
    wsDELETE(m_pDataset);
}

void wxGISVectorPropertyPage::Apply(void)
{
	
}

bool wxGISVectorPropertyPage::CanApply() const
{
	//TODO: Allow to edit spatial reference
	return false;
}

bool wxGISVectorPropertyPage::FillProperties(wxGxSelection* const pSel)
{
	if(m_pg)
	{
		m_pg->Clear();
		
		wsDELETE(m_pDataset);
		if(NULL == pSel)
			return false;
			
		wxGxCatalogBase* pCat = GetGxCatalog();	
		wxGxObject* pGxObject = pCat->GetRegisterObject(pSel->GetFirstSelectedObjectId());	
		if(NULL == pGxObject)
			return false;
			
		m_pGxDataset = wxDynamicCast(pGxObject, wxGxDataset);
		if(NULL == m_pGxDataset)
			return false;

		m_pDataset = wxDynamicCast(m_pGxDataset->GetDataset(false), wxGISFeatureDataset);
		if(!m_pDataset)
			return false;
			
		if(m_pDataset->IsCaching())
			return false;
		
		/*if(m_pDataset->IsOpened() && m_pDataset->IsReadOnly())
		{
			m_pDataset->Close();
			if(!m_pDataset->Open(0, true, true, false))
				return false;
			
		}
		else */if (!m_pDataset->IsOpened())
		{
            if (!m_pDataset->Open(0, true, true, false))
            {
                wsDELETE(m_pDataset);
                return false;
            }
		}	
		FillGrid();	

        wsDELETE(m_pDataset);
	}
	return true;
}			

bool wxGISVectorPropertyPage::Create(ITrackCancel * const pTrackCancel, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
{
    if(!wxPanel::Create(parent, id, pos, size, style, name))
		return false;

	wxBoxSizer* bMainSizer;
	bMainSizer = new wxBoxSizer( wxVERTICAL );

    m_pg = new wxPropertyGrid(this, ID_PPCTRL, wxDefaultPosition, wxDefaultSize, wxPG_DEFAULT_STYLE | wxPG_TOOLTIPS | wxPG_SPLITTER_AUTO_CENTER);
    m_pg->SetColumnProportion(0, 30);
    m_pg->SetColumnProportion(1, 70);

    bMainSizer->Add( m_pg, 1, wxEXPAND | wxALL, 5 );

	this->SetSizerAndFit( bMainSizer );
	this->Layout();

    return true;
}

wxPGProperty* wxGISVectorPropertyPage::AppendProperty(wxPGProperty* pProp)
{
    wxPGProperty* pNewProp = m_pg->Append(pProp);
    pNewProp->ChangeFlag(wxPG_PROP_READONLY, 1);
    return pNewProp;
}

wxPGProperty* wxGISVectorPropertyPage::AppendProperty(wxPGProperty* pid, wxPGProperty* pProp)
{
    wxPGProperty* pNewProp = m_pg->AppendIn(pid, pProp);
    pNewProp->ChangeFlag(wxPG_PROP_READONLY, 1);
    return pNewProp;
}

wxPGProperty* wxGISVectorPropertyPage::AppendMetadataProperty(wxString sMeta)
{
    int nPos = sMeta.Find('=');
    if(nPos == wxNOT_FOUND)
		return AppendProperty( new wxStringProperty(_("Item"), wxString::Format(wxT("Item_%d"), ++m_nCounter), sMeta) );
    else
    {
        wxString sName = sMeta.Left(nPos);
        wxString sVal = sMeta.Right(sMeta.Len() - nPos - 1);
        //clean
        wxString sCleanVal;
        for(size_t i = 0; i < sVal.Len(); ++i)
        {
            char c = sVal.GetChar(i);
            if(c > 31 && c != 127)
                sCleanVal += c;
        }
        if(sCleanVal.Len() > 500)
            return m_pg->Append( new wxLongStringProperty(sName, wxString::Format(wxT("%s_%d"), sName.c_str(), ++m_nCounter), sCleanVal) );//??
        else
            return AppendProperty( new wxStringProperty(sName, wxString::Format(wxT("%s_%d"), sName.c_str(), ++m_nCounter), sCleanVal) );
    }
}

void wxGISVectorPropertyPage::FillGrid(void)
{
    wxString sTmp;
    //fill propertygrid
    wxPGProperty* pid = AppendProperty( new wxPropertyCategory(_("Data Source")) );
    AppendProperty( new wxStringProperty(_("Vector"), wxPG_LABEL, m_pDataset->GetName()) );

    CPLString soPath(m_pDataset->GetPath());

    //folder
	wxString sPath(CPLGetPath(soPath), wxConvUTF8);
    AppendProperty( new wxStringProperty(_("Folder"), wxPG_LABEL, sPath) );

    char** papszFileList = m_pDataset->GetFileList();
    if( !papszFileList || CSLCount(papszFileList) == 0 )
    {
        AppendProperty( new wxStringProperty(_("Files"), wxPG_LABEL, _("None associated")) );
    }
    else
    {
        wxPGProperty* pfilesid = AppendProperty(pid, new wxPropertyCategory(_("Files")) );
        for(int i = 0; papszFileList[i] != NULL; ++i )
	    {
 		    wxString sFileName = GetConvName(papszFileList[i]);
			AppendProperty(pfilesid, new wxStringProperty(wxString::Format(_("File %d"), i), wxPG_LABEL, sFileName) );
	    }
    }
    CSLDestroy( papszFileList );

    //size
    m_pGxDataset->FillMetadata(false);
    //size
    AppendProperty(pid, new wxStringProperty(_("Total size"), wxPG_LABEL, wxFileName::GetHumanReadableSize(m_pGxDataset->GetSize())) );
	if(m_pGxDataset->GetModificationDate().IsValid())
		AppendProperty(pid, new wxStringProperty(_("Modification date"), wxPG_LABEL, m_pGxDataset->GetModificationDate().Format()) );

    OGRCompatibleDataSource *pDataSource = m_pDataset->GetDataSourceRef();
    if(pDataSource)
    {
        OGRCompatibleDriver* pDrv = dynamic_cast<OGRCompatibleDriver*>(pDataSource->GetDriver());
		wxPGProperty* pdriversid = NULL;
		if(pDrv)
			pdriversid = AppendProperty(pid, new wxStringProperty(_("Driver"), wxPG_LABEL, wxString(pDrv->GetOGRCompatibleDriverName(), wxConvUTF8) ));
        OGRSFDriver* pOGRSFDrv = dynamic_cast<OGRSFDriver*>(pDrv);
        if(pOGRSFDrv && pdriversid)
        {            
            //TestCapability
            AppendProperty(pdriversid, new wxStringProperty(_("Create DataSource"), wxPG_LABEL, pOGRSFDrv->TestCapability(ODrCCreateDataSource) == TRUE ? _("true") : _("false")) );
            AppendProperty(pdriversid, new wxStringProperty(_("Delete DataSource"), wxPG_LABEL, pOGRSFDrv->TestCapability(ODrCDeleteDataSource) == TRUE ? _("true") : _("false")) );
        }
        wxPGProperty* pdssid = AppendProperty(pid, new wxStringProperty(_("DataSource"), wxT("DataSource_det"), wxString(pDataSource->GetOGRCompatibleDatasourceName(), wxConvUTF8) ));
        AppendProperty(pdssid, new wxIntProperty(_("Layer Count"), wxPG_LABEL, pDataSource->GetLayerCount()) );
        AppendProperty(pdssid, new wxStringProperty(_("Create Layer"), wxPG_LABEL, pDataSource->TestCapability(ODsCCreateLayer) == TRUE ? _("true") : _("false")) );
        AppendProperty(pdssid, new wxStringProperty(_("Delete Layer"), wxPG_LABEL, pDataSource->TestCapability(ODsCDeleteLayer) == TRUE ? _("true") : _("false")));

#if GDAL_VERSION_NUM >= 1110000
        AppendProperty(pdssid, new wxStringProperty(_("Create Geom Field After Create Layer"), wxPG_LABEL, pDataSource->TestCapability(ODsCCreateGeomFieldAfterCreateLayer) == TRUE ? _("true") : _("false")));
#endif

    }

    if(pDataSource)
    {
        AppendProperty( new wxPropertyCategory(_("Information")) );
        if(m_pGxDataset->GetType() == enumGISContainer)
        //if(pDataSource->GetLayerCount() > 0)
        {
            for( int iLayer = 0; iLayer < pDataSource->GetLayerCount(); ++iLayer )
            {
                OGRLayer *poLayer = pDataSource->GetLayer(iLayer);
                if(poLayer)
                    FillLayerDef(poLayer, iLayer, soPath);
            }
        }
        else
        {
            OGRLayer *poLayer = m_pDataset->GetLayerRef();
            if(poLayer)
                FillLayerDef(poLayer, 0, soPath);
        }
    }
}

void wxGISVectorPropertyPage::FillLayerDef(OGRLayer *poLayer, int iLayer, CPLString soPath)
{
    wxPGProperty* playid = AppendProperty( new wxPropertyCategory(wxString::Format(_("Layer #%d"), iLayer + 1) ));

    wxString sOut = GetConvName(poLayer->GetName(), false);
	if(sOut.IsEmpty())
	{
        sOut = GetConvName(CPLGetBasename(soPath), false);
	}

	AppendProperty(playid, new wxStringProperty(_("Name"), wxString::Format(wxT("Name_%d"), iLayer), sOut));  //GetConvName
    AppendProperty(playid, new wxStringProperty(_("Geometry type"), wxString::Format(wxT("Geometry type_%d"), iLayer), wxString(OGRGeometryTypeToName( poLayer->GetGeomType() ), wxConvLocal)));

	AppendProperty(playid, new wxIntProperty(_("Feature count"), wxString::Format(wxT("Feature count_%d"), iLayer), poLayer->GetFeatureCount() ));
	
    wxString sFidCol = wxString::FromUTF8(poLayer->GetFIDColumn()); //m_pDataset->GetFIDColumn();
    if (!sFidCol.IsEmpty())
        AppendProperty(playid, new wxStringProperty(_("FID Column"), wxString::Format(wxT("FID Column_%d"), iLayer), sFidCol));

    if( CPLStrnlen(poLayer->GetGeometryColumn(), 100) > 0 )
        AppendProperty(playid, new wxStringProperty(_("Geometry Column"), wxString::Format(wxT("Geometry Column_%d"), iLayer), wxString( poLayer->GetGeometryColumn(), wxConvLocal )));
	wxString sEncodingDesc = wxFontMapper::GetEncodingDescription(m_pDataset->GetEncoding());
	AppendProperty(playid, new wxStringProperty(_("Encoding"), wxString::Format(wxT("Encoding_%d"), iLayer), sEncodingDesc));

    OGRFeatureDefn* const poDefn = poLayer->GetLayerDefn();
    if(poDefn)
    {
        wxPGProperty* pfieldsid = AppendProperty(playid, new wxPropertyCategory(wxString::Format(_("Layer #%d Fields (%d)"), iLayer + 1, poDefn->GetFieldCount())) );
        for( int iAttr = 0; iAttr < poDefn->GetFieldCount(); iAttr++ )
        {
            OGRFieldDefn    *poField = poDefn->GetFieldDefn( iAttr );
            wxString sFieldTypeName = wxString( poField->GetFieldTypeName( poField->GetType() ), wxConvLocal );
			wxPGProperty* pfielid = AppendProperty(pfieldsid, new wxStringProperty(_("Name"), wxString::Format(wxT("Name_%d_%d"), iLayer, iAttr), wxString::Format(wxT("%s (%s)"), wxString(poField->GetNameRef(), wxCSConv(m_pDataset->GetEncoding())).c_str(), sFieldTypeName.c_str()) ));
            AppendProperty(pfielid, new wxStringProperty(_("Type"), wxString::Format(wxT("Type_%d_%d"), iLayer, iAttr), sFieldTypeName ) );
            AppendProperty(pfielid, new wxIntProperty(_("Width"), wxString::Format(wxT("Width_%d_%d"), iLayer, iAttr), poField->GetWidth()) );
            AppendProperty(pfielid, new wxIntProperty(_("Precision"), wxString::Format(wxT("Precision_%d_%d"), iLayer, iAttr), poField->GetPrecision()) );
            OGRJustification Just = poField->GetJustify();
            wxString sJust(_("Undefined"));
            switch(Just)
            {
            case OJLeft:
                sJust = wxString(_("Left"));
                break;
            case OJRight:
                sJust = wxString(_("Right"));
                break;
            }
            AppendProperty(pfielid, new wxStringProperty(_("Justify"), wxString::Format(wxT("Justify_%d_%d"), iLayer, iAttr), sJust) );
            m_pg->Collapse(pfielid);

        }
    }
    //TestCapability
    wxPGProperty* pcapid = AppendProperty(playid, new wxPropertyCategory(wxString::Format(_("Layer #%d Capability"), iLayer + 1)) );
    AppendProperty(pcapid, new wxStringProperty(_("Random Read"), wxString::Format(wxT("Random Read_%d"), iLayer), poLayer->TestCapability(OLCRandomRead) == TRUE ? _("true") : _("false")));
    AppendProperty(pcapid, new wxStringProperty(_("Sequential Write"), wxString::Format(wxT("Sequential Write_%d"), iLayer), poLayer->TestCapability(OLCSequentialWrite) == TRUE ? _("true") : _("false")));
    AppendProperty(pcapid, new wxStringProperty(_("Random Write"), wxString::Format(wxT("Random Write_%d"), iLayer), poLayer->TestCapability(OLCRandomWrite) == TRUE ? _("true") : _("false")));
    AppendProperty(pcapid, new wxStringProperty(_("Fast Spatial Filter"), wxString::Format(wxT("Fast Spatial Filter_%d"), iLayer), poLayer->TestCapability(OLCFastSpatialFilter) == TRUE ? _("true") : _("false")));
    AppendProperty(pcapid, new wxStringProperty(_("Fast Feature Count"), wxString::Format(wxT("Fast Feature Count_%d"), iLayer), poLayer->TestCapability(OLCFastFeatureCount) == TRUE ? _("true") : _("false")));
    AppendProperty(pcapid, new wxStringProperty(_("Fast Get Extent"), wxString::Format(wxT("Fast Get Extent_%d"), iLayer), poLayer->TestCapability(OLCFastGetExtent) == TRUE ? _("true") : _("false")));
    AppendProperty(pcapid, new wxStringProperty(_("Fast Set Next By Index"), wxString::Format(wxT("Fast Set Next By Index_%d"), iLayer), poLayer->TestCapability(OLCFastSetNextByIndex) == TRUE ? _("true") : _("false")));
    AppendProperty(pcapid, new wxStringProperty(_("Create Field"), wxString::Format(wxT("Create Field_%d"), iLayer), poLayer->TestCapability(OLCCreateField) == TRUE ? _("true") : _("false")));
    AppendProperty(pcapid, new wxStringProperty(_("Delete Field"), wxString::Format(wxT("Delete Field_%d"), iLayer), poLayer->TestCapability(OLCDeleteField) == TRUE ? _("true") : _("false")));
    AppendProperty(pcapid, new wxStringProperty(_("Reorder Fields"), wxString::Format(wxT("Reorder Fields_%d"), iLayer), poLayer->TestCapability(OLCReorderFields) == TRUE ? _("true") : _("false")));
    AppendProperty(pcapid, new wxStringProperty(_("Alter Field Definition"), wxString::Format(wxT("Alter Field Definition_%d"), iLayer), poLayer->TestCapability(OLCAlterFieldDefn) == TRUE ? _("true") : _("false")));

#if GDAL_VERSION_NUM >= 1110000
        AppendProperty(pcapid, new wxStringProperty(_("Create Geometry Field"), wxString::Format(wxT("Create Geometry Field_%d"), iLayer), poLayer->TestCapability(OLCCreateGeomField) == TRUE ? _("true") : _("false")));
#endif

    AppendProperty(pcapid, new wxStringProperty(_("Delete Feature"), wxString::Format(wxT("Delete Feature_%d"), iLayer), poLayer->TestCapability(OLCDeleteFeature) == TRUE ? _("true") : _("false")));
    AppendProperty(pcapid, new wxStringProperty(_("Strings As UTF8"), wxString::Format(wxT("Strings As UTF8_%d"), iLayer), poLayer->TestCapability(OLCStringsAsUTF8) == TRUE ? _("true") : _("false")));
    AppendProperty(pcapid, new wxStringProperty(_("Transactions"), wxString::Format(wxT("Transactions_%d"), iLayer), poLayer->TestCapability(OLCTransactions) == TRUE ? _("true") : _("false")));
    AppendProperty(pcapid, new wxStringProperty(_("Ignore Fields"), wxString::Format(wxT("Ignore Fields_%d"), iLayer), poLayer->TestCapability(OLCIgnoreFields) == TRUE ? _("true") : _("false")));

    OGREnvelope Extent;
    OGRErr eErr = poLayer->GetExtent(&Extent);
    if(eErr == OGRERR_NONE)
    {
        OGRSpatialReference* pSpaRef = poLayer->GetSpatialRef();
        wxGISSpatialReference SpaRef;
        bool bProjected(false);
        if(pSpaRef)
        {
            SpaRef = wxGISSpatialReference(pSpaRef->Clone());
            if(SpaRef.IsOk() && SpaRef->IsProjected())
                bProjected = true;
        }
        wxPGProperty* penvid(NULL);
        if(bProjected)
            penvid = AppendProperty(playid, new wxPropertyCategory(wxString::Format(_("Layer #%d Extent %s"), iLayer + 1, _("(Projected)"))));
        else
            penvid = AppendProperty(playid, new wxPropertyCategory(wxString::Format(_("Layer #%d Extent %s"), iLayer + 1, wxEmptyString)));
        AppendProperty(penvid, new wxFloatProperty(_("Top"), wxString::Format(wxT("Top_%d"), iLayer), Extent.MaxY));
        AppendProperty(penvid, new wxFloatProperty(_("Left"), wxString::Format(wxT("Left_%d"), iLayer), Extent.MinX));
        AppendProperty(penvid, new wxFloatProperty(_("Bottom"), wxString::Format(wxT("Bottom_%d"), iLayer), Extent.MinY));
        AppendProperty(penvid, new wxFloatProperty(_("Right"), wxString::Format(wxT("Right_%d"), iLayer), Extent.MaxX));

        AppendProperty(penvid, new wxFloatProperty(_("Center X"), wxString::Format(wxT("CenterX_%d"), iLayer), (Extent.MaxX + Extent.MinX) / 2.0));
        AppendProperty(penvid, new wxFloatProperty(_("Center Y"), wxString::Format(wxT("CenterY_%d"), iLayer), (Extent.MaxY + Extent.MinY) / 2.0));

        if(bProjected)
        {
            wxGISSpatialReference GSSpaRef(SpaRef->CloneGeogCS());
            if(GSSpaRef.IsOk())
            {
                OGRCoordinateTransformation *poCT = OGRCreateCoordinateTransformation( SpaRef, GSSpaRef );
                if(poCT)
                {
                    poCT->Transform(1, &Extent.MaxX, &Extent.MaxY);
                    poCT->Transform(1, &Extent.MinX, &Extent.MinY);
                    wxPGProperty* penvidgc = AppendProperty(playid, new wxPropertyCategory(wxString::Format(_("Layer #%d Extent %s"), iLayer + 1, _("(Geographic)"))));
                    AppendProperty(penvidgc, new wxFloatProperty(_("Top"), wxString::Format(wxT("TopGC_%d"), iLayer), Extent.MaxY));
                    AppendProperty(penvidgc, new wxFloatProperty(_("Left"), wxString::Format(wxT("LeftGC_%d"), iLayer), Extent.MinX));
                    AppendProperty(penvidgc, new wxFloatProperty(_("Bottom"), wxString::Format(wxT("BottomGC_%d"), iLayer), Extent.MinY));
                    AppendProperty(penvidgc, new wxFloatProperty(_("Right"), wxString::Format(wxT("RightGC_%d"), iLayer), Extent.MaxX));

                    AppendProperty(penvidgc, new wxFloatProperty(_("Center X"), wxString::Format(wxT("CenterXGC_%d"), iLayer), (Extent.MaxX + Extent.MinX) / 2.0));
                    AppendProperty(penvidgc, new wxFloatProperty(_("Center Y"), wxString::Format(wxT("CenterYGC_%d"), iLayer), (Extent.MaxY + Extent.MinY) / 2.0));

                    OCTDestroyCoordinateTransformation(poCT);
                }
            }
        }
    }

    //OGREnvelope Extent;
    //if(poLayer->GetExtent(&Extent, true) == OGRERR_NONE)
    //{
    //    wxPGProperty* penvid = AppendProperty(playid, new wxPropertyCategory(wxString::Format(_("Layer #%d Extent"), iLayer + 1)));
    //    AppendProperty(penvid, new wxFloatProperty(_("Top"), wxString::Format(wxT("Top_%d"), iLayer), Extent.MaxY));
    //    AppendProperty(penvid, new wxFloatProperty(_("Left"), wxString::Format(wxT("Left_%d"), iLayer), Extent.MinX));
    //    AppendProperty(penvid, new wxFloatProperty(_("Right"), wxString::Format(wxT("Right_%d"), iLayer), Extent.MaxX));
    //    AppendProperty(penvid, new wxFloatProperty(_("Bottom"), wxString::Format(wxT("Bottom_%d"), iLayer), Extent.MinY));
    //}
}

void wxGISVectorPropertyPage::OnChildFocus( wxChildFocusEvent& event )
{
	// do nothing to avoid "scrollbar jump" if wx2.9 is used
}
