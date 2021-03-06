/******************************************************************************
 * Project:  wxGIS (GIS Toolbox)
 * Purpose:  controls classes.
 * Author:   Dmitry Baryshnikov (aka Bishop), polimax@mail.ru
 ******************************************************************************
*   Copyright (C) 2009-2012,2014 Dmitry Baryshnikov
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
#include "wxgis/geoprocessingui/gpcontrols.h"
#include "wxgis/catalogui/gxobjdialog.h"
#include "wxgis/catalog/gxdataset.h"
#include "wxgis/datasource/table.h"
#include "wxgis/datasource/featuredataset.h"
#include "wxgis/framework/dataobject.h"
#include "wxgis/catalogui/droptarget.h"
#include "wxgis/catalogui/gxcontdialog.h"
#include "wxgis/catalogui/gxfileui.h"

/*
#include "wxgis/geoprocessing/gpdomain.h"
#include "wxgis/geoprocessing/gpparam.h"
#include "wxgis/catalogui/gxobjdialog.h"
#include "wxgis/cartoui/sqlquerydialog.h"

#include "wx/dnd.h"
#include "wx/valgen.h"
#include "wx/tokenzr.h"
//#include "wx/propgrid/props.h"
#include "wx/propgrid/propgrid.h"

#include "../../art/add_to_list.xpm"
#include "../../art/querysql.xpm"
*/
#include <wx/clipbrd.h>

#include "../../art/state.xpm"
#include "../../art/open.xpm"

//-------------------------------------------------------------------------------
// Class wxGISDTBase
//-------------------------------------------------------------------------------

IMPLEMENT_ABSTRACT_CLASS(wxGISDTBase, wxPanel)

BEGIN_EVENT_TABLE(wxGISDTBase, wxPanel)
	EVT_GPPARAM_MSG_SET( wxGISDTBase::OnParamMsgSet )
	EVT_GPPARAM_CHANGED( wxGISDTBase::OnParamChanged )
END_EVENT_TABLE()

wxGISDTBase::wxGISDTBase(  const wxGISGPParameterArray &Params, int nParamIndex, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxPanel( parent, id, pos, size, style )
{
    m_pParam = Params[nParamIndex];
    m_nAdvCookie = m_pParam->Advise(this);

    m_Params = Params;
    m_nParamIndex = nParamIndex;

	m_ImageList.Create(16, 16);
	m_ImageList.Add(wxBitmap(state_xpm));
}

wxGISDTBase::~wxGISDTBase()
{
}

void wxGISDTBase::OnDelete()
{
    if (m_nAdvCookie != wxNOT_FOUND && NULL != m_pParam)
    {
        m_pParam->Unadvise(m_nAdvCookie);
        m_nAdvCookie = wxNOT_FOUND;
    }    
}

wxGISGPParameter* wxGISDTBase::GetParameter(void) const
{
    return m_Params[m_nParamIndex];
}

void wxGISDTBase::OnParamMsgSet(wxGISGPParamEvent &event)
{
    if(event.GetId() == m_nParamIndex)
    {
        SetMessage(m_pParam->GetMessageType(), m_pParam->GetMessage());
    }
}

void wxGISDTBase::SetMessage(wxGISEnumMessageType nType, const wxString &sMsg)
{
	if(m_nCurrentType == nType && m_sCurrentMsg == sMsg)
		return;
	m_nCurrentType = nType;
	m_sCurrentMsg = sMsg;
    switch(nType)
    {
    case enumGISMessageInformation:
        m_StateBitmap->SetBitmap(m_ImageList.GetIcon(0));
        break;
    case enumGISMessageError:
        m_StateBitmap->SetBitmap(m_ImageList.GetIcon(2));
        break;
    case enumGISMessageWarning:
        m_StateBitmap->SetBitmap(m_ImageList.GetIcon(3));
        break;
    case enumGISMessageRequired:
        m_StateBitmap->SetBitmap(m_ImageList.GetIcon(4));
        break;
    case enumGISMessageOk:
        m_StateBitmap->SetBitmap(m_ImageList.GetIcon(1));
        break;
    case enumGISMessageNone:
        m_StateBitmap->SetBitmap(wxNullBitmap);
        break;
    default:
    case enumGISMessageUnknown:
        m_StateBitmap->SetBitmap(wxNullBitmap);
        break;
    }
    m_StateBitmap->SetToolTip(sMsg);
}

//----------------------------------------------------------------------------------
// Class wxGISDTPath
//----------------------------------------------------------------------------------

IMPLEMENT_CLASS(wxGISDTPath, wxGISDTBase)

BEGIN_EVENT_TABLE(wxGISDTPath, wxGISDTBase)
	EVT_BUTTON(wxID_OPEN, wxGISDTPath::OnOpen)
	EVT_UPDATE_UI(ID_PATHCTRL, wxGISDTPath::OnUpdateUI)
	EVT_TEXT(ID_PATHCTRL, wxGISDTPath::OnPathChange)
END_EVENT_TABLE()

wxGISDTPath::wxGISDTPath(  const wxGISGPParameterArray &Params, int nParamIndex, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxGISDTBase( Params, nParamIndex, parent, id, pos, size, style )
{
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizer1->AddGrowableCol( 1 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_StateBitmap = new wxStaticBitmap( this, wxID_ANY, m_pParam->GetParameterType() == enumGISGPParameterTypeRequired ? m_ImageList.GetIcon(4) : wxNullBitmap , wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( m_StateBitmap, 0, wxALL, 5 );

	m_sFullDisplayName = m_pParam->GetParameterType() == enumGISGPParameterTypeOptional ? m_pParam->GetDisplayName() + wxT(" ") + _("(optional)") : m_pParam->GetDisplayName();
    m_sParamDisplayName = new wxStaticText( this, wxID_ANY, wxT("..."), wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_END );
	m_sParamDisplayName->SetToolTip(m_sFullDisplayName);
	fgSizer1->Add( m_sParamDisplayName, 1, wxALL|wxEXPAND, 5 );

	m_bitmap = new wxStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( m_bitmap, 0, wxALL, 5 );

	wxBoxSizer* bPathSizer;
	bPathSizer = new wxBoxSizer( wxHORIZONTAL );

    m_PathTextCtrl = new wxTextCtrl( this, ID_PATHCTRL, GetParameter()->GetValue(), wxDefaultPosition, wxDefaultSize, wxTE_CHARWRAP );
    m_PathTextCtrl->DragAcceptFiles(true);
    m_PathTextCtrl->SetDropTarget(new wxGISDropTarget(static_cast<IViewDropTarget*>(this)));

    //m_PathTextCtrl->Connect(wxEVT_DROP_FILES, wxDropFilesEventHandler(wxGISSelectTemplatePage::OnDropFiles), NULL, this);
    bPathSizer->Add(m_PathTextCtrl, 1, wxALL | wxEXPAND, 5);

	m_bpButton = new wxBitmapButton( this, wxID_OPEN, wxBitmap(open_xpm), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	bPathSizer->Add( m_bpButton, 0, wxALL, 5 );
	fgSizer1->Add( bPathSizer, 0, wxALL|wxEXPAND, 5 );

	this->SetSizer( fgSizer1 );
	this->Layout();

	m_sParamDisplayName->SetLabel(m_sFullDisplayName);
}

wxGISDTPath::~wxGISDTPath()
{
}

void wxGISDTPath::OnOpen(wxCommandEvent& event)
{
    wxGISGPGxObjectDomain* pDomain = wxDynamicCast(m_pParam->GetDomain(), wxGISGPGxObjectDomain);
    wxFileName Name(m_pParam->GetValue().MakeString());

    if(m_pParam->GetDirection() == enumGISGPParameterDirectionInput)
    {
        wxGxObjectDialog dlg(this, wxID_ANY, _("Select input object"));

        wxGISAppConfig oConfig = GetConfig();
        if(oConfig.IsOk())
        {
            wxString sPath = oConfig.Read(enumGISHKCU, dlg.GetAppName() + wxT("/lastpath/") + m_pParam->GetName() + wxT("/path"), wxEmptyString);
            if(!sPath.IsEmpty())
                dlg.SetStartingLocation(sPath);
        }
        dlg.SetAllowMultiSelect(false);
        dlg.SetAllFilters(true);
        dlg.SetOwnsFilter(false);
        dlg.SetName( Name.GetFullName() );
        if(pDomain)
        {
            for(size_t i = 0; i < pDomain->GetCount(); ++i)
            {
				if(i == m_pParam->GetSelDomainValue())
                    dlg.AddFilter(pDomain->GetFilter(i), true);
                else
                    dlg.AddFilter(pDomain->GetFilter(i), false);
            }
        }
        dlg.SetOverwritePrompt(false);
        if(dlg.ShowModalOpen() == wxID_OK)
        {
            wxString sPath = dlg.GetFullName();

			m_pParam->SetSelDomainValue(dlg.GetCurrentFilterId());
            m_pParam->SetAltered(true);
            m_pParam->SetValue(wxVariant(sPath, wxT("path")));

            if(oConfig.IsOk() && dlg.GetLocation())
            {
                wxString sContPath = dlg.GetLocation()->GetFullName();
                oConfig.Write(enumGISHKCU, wxT("wxGISObjDialog/lastpath/") + m_pParam->GetName() + wxT("/path"), sContPath);
            }
        }
    }
    else
    {
        wxGxObjectDialog dlg(this, wxID_ANY, _("Select output object"));
        wxGISAppConfig oConfig = GetConfig();
        if(oConfig.IsOk())
        {
            wxString sPath = oConfig.Read(enumGISHKCU, wxT("wxGISObjDialog/lastpath/") + m_pParam->GetName() + wxT("/path"), wxEmptyString);
            if(!sPath.IsEmpty())
                dlg.SetStartingLocation(sPath);
        }

        dlg.SetAllowMultiSelect(false);
        dlg.SetAllFilters(false);
        dlg.SetOwnsFilter(false);
        dlg.SetName( Name.GetFullName() );
        if(pDomain)
        {
            for(size_t i = 0; i < pDomain->GetCount(); ++i)
            {
				if(i == m_pParam->GetSelDomainValue())
                    dlg.AddFilter(pDomain->GetFilter(i), true);
                else
                    dlg.AddFilter(pDomain->GetFilter(i), false);
            }
        }
        dlg.SetOverwritePrompt(false);
        if(dlg.ShowModalSave() == wxID_OK)
        {
            wxString sPath = dlg.GetFullName();

            wxString sExt = pDomain->GetFilter(m_pParam->GetSelDomainValue())->GetExt();
            wxFileName FName(sPath);
            FName.SetExt(sExt);
            sPath = FName.GetFullPath();

            m_pParam->SetAltered(true);
			m_pParam->SetSelDomainValue(dlg.GetCurrentFilterId());
            m_pParam->SetValue(wxVariant(sPath, wxT("path")));

            if(oConfig.IsOk() && dlg.GetLocation())
            {
                wxString sContPath = dlg.GetLocation()->GetFullName();
                oConfig.Write(enumGISHKCU, wxT("wxGISObjDialog/lastpath/") + m_pParam->GetName() + wxT("/path"), sContPath);
            }
        }
    }
}

//validate
bool wxGISDTPath::Validate(void)
{
    if(m_pParam->GetHasBeenValidated())
		return m_pParam->IsValid();

	m_pParam->SetHasBeenValidated(true);
    wxString sPath = m_pParam->GetValue();
    if(sPath.IsEmpty())
    {
        m_pParam->SetAltered(false);
        if(m_pParam->GetParameterType() != enumGISGPParameterTypeRequired)
        {
            m_pParam->SetValid(true);
            m_pParam->SetMessage(enumGISMessageNone);
            return true;
        }
        else
        {
            m_pParam->SetValid(false);
            m_pParam->SetMessage(enumGISMessageRequired, _("The value is required"));
            return false;
        }
    }

    wxGxCatalogBase* pCatalog = GetGxCatalog();
    if(pCatalog)
    {
        wxGxObject* pGxObj = pCatalog->FindGxObject(sPath);
        if(pGxObj)
        {
           if(m_pParam->GetDirection() == enumGISGPParameterDirectionInput)
           {
               m_pParam->SetValid(true);
               m_pParam->SetMessage(enumGISMessageOk);
           }
           else
           {
               m_pParam->SetValid(true);
               m_pParam->SetMessage(enumGISMessageWarning, _("The output object exists and will be overwritten!"));
           }
           return true;
        }
        else
        {
           if(m_pParam->GetDirection() == enumGISGPParameterDirectionInput)
           {
                m_pParam->SetValid(false);
                m_pParam->SetMessage(enumGISMessageError, _("The input object doesn't exist"));
                return false;
           }
           else
           {
               m_pParam->SetValid(true);
               m_pParam->SetMessage(enumGISMessageOk);
               return true;
           }
        }
    }
    return true;
}

void wxGISDTPath::OnUpdateUI(wxUpdateUIEvent &event)
{
	Validate();
}

void wxGISDTPath::OnPathChange(wxCommandEvent& event)
{
	wxString sData = m_PathTextCtrl->GetValue();
    wxGISGPGxObjectDomain* poDomain = wxDynamicCast(m_pParam->GetDomain(), wxGISGPGxObjectDomain);
    if(poDomain)
    {
        wxFileName oName(sData);
        if(oName.IsOk())
        {
            if(oName.HasExt()) //set sel domain
            {
		        for(size_t i = 0; i < poDomain->GetCount(); ++i)
		        {
			        wxGxObjectFilter* poFilter = poDomain->GetFilter(i);
			        if(poFilter)
			        {
                        if(oName.GetExt().IsSameAs(poFilter->GetExt(), false) || poFilter->GetExt().IsEmpty())
				        {
					        m_pParam->SetSelDomainValue(i);
					        break;
				        }
			        }
		        }
            }
            else //set ext by sel domain
            {
                wxGxObjectFilter* poFilter = poDomain->GetFilter(m_pParam->GetSelDomainValue());
                oName.SetExt(poFilter->GetExt());
                sData = oName.GetFullPath();
            }
        }
    }
    m_pParam->SetAltered(true);
    m_pParam->SetValue(wxVariant(sData, wxT("path")));
}

void wxGISDTPath::OnParamChanged(wxGISGPParamEvent& event)
{
    if(event.GetId() == m_nParamIndex)
    {
        m_PathTextCtrl->ChangeValue( event.GetParamValue() );
        Validate();
    }
}


wxDragResult wxGISDTPath::OnDragOver(wxCoord x, wxCoord y, wxDragResult def)
{
    return def;
}

bool wxGISDTPath::OnDropObjects(wxCoord x, wxCoord y, const wxArrayString& GxObjects, bool bIsControlOn, bool bIsShiftOn)
{
    if (GxObjects.GetCount() > 0)
    {
        m_PathTextCtrl->ChangeValue(GxObjects[0]);
    }

    return true;
}

void wxGISDTPath::OnLeave()
{
}

bool wxGISDTPath::CanPaste()
{
    wxClipboardLocker lockClip;
    return wxTheClipboard->IsSupported(wxDF_FILENAME) | wxTheClipboard->IsSupported(wxDataFormat(wxGIS_DND_NAME));
    //& wxTheClipboard->IsSupported(wxDF_TEXT); | wxDF_BITMAP | wxDF_TIFF | wxDF_DIB | wxDF_UNICODETEXT | wxDF_HTML
}

//----------------------------------------------------------------------------
// Class wxGISDTFolderPath
//----------------------------------------------------------------------------

IMPLEMENT_CLASS(wxGISDTFolderPath, wxGISDTPath)


wxGISDTFolderPath::wxGISDTFolderPath( const wxGISGPParameterArray &Params, int nParamIndex, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxGISDTPath( Params, nParamIndex, parent, id, pos, size, style )
{
}

wxGISDTFolderPath::~wxGISDTFolderPath()
{
}

void wxGISDTFolderPath::OnOpen(wxCommandEvent& event)
{
    wxGISGPGxObjectDomain* pDomain = dynamic_cast<wxGISGPGxObjectDomain*>(m_pParam->GetDomain());
    wxString sCurrentPath(m_pParam->GetValue().MakeString());

    wxGxContainerDialog dlg(this, wxID_ANY, m_pParam->GetDirection() == enumGISGPParameterDirectionInput ? _("Select input object") : _("Select output object"));
	dlg.SetButtonCaption(_("Select"));
	dlg.ShowCreateButton(true);
    dlg.SetOwnsFilter(false);
    dlg.SetOwnsShowFilter(false);
	dlg.ShowExportFormats();
    if(pDomain)
    {
        for(size_t i = 0; i < pDomain->GetCount(); ++i)
        {
			if(i == m_pParam->GetSelDomainValue())
                dlg.AddFilter(pDomain->GetFilter(i), true);
            else
                dlg.AddFilter(pDomain->GetFilter(i), false);
			dlg.AddShowFilter(pDomain->GetFilter(i));
        }
        dlg.SetAllFilters(false);
    }
    if (!sCurrentPath.IsEmpty())
        dlg.SetStartingLocation(sCurrentPath);

    if(dlg.ShowModal() == wxID_OK)
    {
        wxString sPath = dlg.GetFullName();
        m_pParam->SetValue(wxVariant(sPath, wxT("path")));
    }

    m_pParam->SetAltered(true);
    m_pParam->SetHasBeenValidated(false);
}

//validate
bool wxGISDTFolderPath::Validate(void)
{
    if(m_pParam->GetHasBeenValidated())
		return m_pParam->IsValid();

	m_pParam->SetHasBeenValidated(true);
    wxString sPath = m_pParam->GetValue();
    if(sPath.IsEmpty())
    {
        m_pParam->SetAltered(false);
        if(m_pParam->GetParameterType() != enumGISGPParameterTypeRequired)
        {
            m_pParam->SetValid(true);
            m_pParam->SetMessage(enumGISMessageNone);
            return true;
        }
        else
        {
            m_pParam->SetValid(false);
            m_pParam->SetMessage(enumGISMessageRequired, _("The value is required"));
            return false;
        }
    }

    wxGxCatalogBase* pCatalog = GetGxCatalog();
    if(pCatalog)
    {
        wxGxObjectContainer* pGxObj = wxDynamicCast(pCatalog->FindGxObject(sPath), wxGxObjectContainer);
        if(pGxObj)
        {
           if(m_pParam->GetDirection() == enumGISGPParameterDirectionInput)
           {
               m_pParam->SetValid(true);
               m_pParam->SetMessage(enumGISMessageOk);
           }
           else
           {
               m_pParam->SetValid(true);
               m_pParam->SetMessage(enumGISMessageWarning, _("The output object exists and will be overwritten!"));
           }
           return true;
        }
        else
        {
           if(m_pParam->GetDirection() == enumGISGPParameterDirectionInput)
           {
                m_pParam->SetValid(false);
                m_pParam->SetMessage(enumGISMessageError, _("The input object doesn't exist"));
                return false;
           }
           else
           {
               m_pParam->SetValid(true);
               m_pParam->SetMessage(enumGISMessageOk);
               return true;
           }
        }
    }
    return true;
}

//----------------------------------------------------------------------------
// Class wxGISDTDigit
//----------------------------------------------------------------------------

IMPLEMENT_CLASS(wxGISDTDigit, wxGISDTBase)

BEGIN_EVENT_TABLE(wxGISDTDigit, wxGISDTBase)
	EVT_UPDATE_UI(ID_DIGITCTRL, wxGISDTDigit::OnUpdateUI)
	EVT_TEXT(ID_DIGITCTRL, wxGISDTDigit::OnDigitChange)
END_EVENT_TABLE()

wxGISDTDigit::wxGISDTDigit( const wxGISGPParameterArray &Params, int nParamIndex, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxGISDTBase( Params, nParamIndex, parent, id, pos, size, style )
{
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizer1->AddGrowableCol( 1 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_StateBitmap = new wxStaticBitmap( this, wxID_ANY, m_pParam->GetParameterType() == enumGISGPParameterTypeRequired ? m_ImageList.GetIcon(4) : wxNullBitmap , wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( m_StateBitmap, 0, wxALL, 5 );

	m_sFullDisplayName = m_pParam->GetParameterType() == enumGISGPParameterTypeOptional ? m_pParam->GetDisplayName() + wxT(" ") + _("(optional)") : m_pParam->GetDisplayName();
    m_sParamDisplayName = new wxStaticText( this, wxID_ANY, wxT("..."), wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_END );
	m_sParamDisplayName->SetToolTip(m_sFullDisplayName);
	fgSizer1->Add( m_sParamDisplayName, 1, wxALL|wxEXPAND, 5 );

	m_bitmap = new wxStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( m_bitmap, 0, wxALL, 5 );

	wxBoxSizer* bPathSizer;
	bPathSizer = new wxBoxSizer( wxHORIZONTAL );

    m_DigitTextCtrl = new wxTextCtrl( this, ID_DIGITCTRL, m_pParam->GetValue(), wxDefaultPosition, wxDefaultSize, wxTE_RIGHT  );
    //m_PathTextCtrl->SetDropTarget(new wxFileDropTarget());
	bPathSizer->Add( m_DigitTextCtrl, 1, wxALL|wxEXPAND, 5 );

	fgSizer1->Add( bPathSizer, 0, wxALL|wxEXPAND, 5 );

	this->SetSizer( fgSizer1 );
	this->Layout();

	m_sParamDisplayName->SetLabel(m_sFullDisplayName);
}

wxGISDTDigit::~wxGISDTDigit()
{
}

bool wxGISDTDigit::Validate(void)
{
    if(m_pParam->GetHasBeenValidated())
		return m_pParam->IsValid();

 	m_pParam->SetHasBeenValidated(true);
    wxString sData = m_pParam->GetValue();

    if(sData.IsEmpty())
    {
        m_pParam->SetAltered(false);
        if(m_pParam->GetParameterType() == enumGISGPParameterTypeOptional)
            m_pParam->SetValid(true);
        return true;
    }

    bool bValid(false);
    long dVal(0);
    double dfVal(0);
    switch(m_pParam->GetDataType())
    {
    case enumGISGPParamDTInteger:
        bValid = sData.ToLong(&dVal);
        break;
    case enumGISGPParamDTDouble:
        bValid = sData.ToDouble(&dfVal);
        break;
    default:
        bValid = false;
        break;
    }

    m_pParam->SetValid(bValid);
    if ( bValid )
    {
        m_pParam->SetMessage(enumGISMessageOk);
        switch(m_pParam->GetDataType())
        {
        case enumGISGPParamDTInteger:
        case enumGISGPParamDTDouble:
        default:
            break;
        }
    }
    else
    {
        m_pParam->SetMessage(enumGISMessageError, _("The input data are invalid"));
        return false;
    }
    return true;
}

void wxGISDTDigit::OnUpdateUI(wxUpdateUIEvent &event)
{
	Validate();
}

void wxGISDTDigit::OnDigitChange(wxCommandEvent& event)
{
	wxString sData = m_DigitTextCtrl->GetValue();
    bool bValid(false);
    long dVal(0);
    double dfVal(0);
    switch(m_pParam->GetDataType())
    {
    case enumGISGPParamDTInteger:
        bValid = sData.ToLong(&dVal);
        break;
    case enumGISGPParamDTDouble:
        bValid = sData.ToDouble(&dfVal);
        break;
    default:
        bValid = true;
        break;
    }

    m_pParam->SetValid(bValid);

    if ( bValid )
    {
        m_pParam->SetAltered(true);
        switch(m_pParam->GetDataType())
        {
        case enumGISGPParamDTInteger:
            m_pParam->SetValue(wxVariant(dVal, wxT("long")));
            return;
        case enumGISGPParamDTDouble:
            m_pParam->SetValue(wxVariant(dfVal, wxT("double")));
            return;
        default:
            m_pParam->SetValue(wxVariant(sData, wxT("val")));
            return;
        }
    }
}

void wxGISDTDigit::OnParamChanged(wxGISGPParamEvent& event)
{
    if(event.GetId() == m_nParamIndex)
    {
        m_DigitTextCtrl->ChangeValue( event.GetParamValue() );
        Validate();
    }
}

//-------------------------------------------------------------------------------
// Class wxGISDTChoice
//-------------------------------------------------------------------------------

IMPLEMENT_CLASS(wxGISDTChoice, wxGISDTBase)

BEGIN_EVENT_TABLE(wxGISDTChoice, wxGISDTBase)
	EVT_CHOICE(ID_CHOICESTR, wxGISDTChoice::OnChoice)
	EVT_UPDATE_UI(ID_CHOICESTR, wxGISDTChoice::OnUpdateUI)
	EVT_GPPARAM_DOMAIN_ADDVAL(wxGISDTChoice::OnParamDomAddVal)
	EVT_GPPARAM_DOMAIN_CLEAR(wxGISDTChoice::OnParamDomClear)
END_EVENT_TABLE()

wxGISDTChoice::wxGISDTChoice( const wxGISGPParameterArray &Params, int nParamIndex, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxGISDTBase( Params, nParamIndex, parent, id, pos, size, style )
{
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizer1->AddGrowableCol( 1 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_StateBitmap = new wxStaticBitmap( this, wxID_ANY, m_pParam->GetParameterType() == enumGISGPParameterTypeRequired ? m_ImageList.GetIcon(4) : wxNullBitmap , wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( m_StateBitmap, 0, wxALL, 5 );

	m_sFullDisplayName = m_pParam->GetParameterType() == enumGISGPParameterTypeOptional ? m_pParam->GetDisplayName() + wxT(" ") + _("(optional)") : m_pParam->GetDisplayName();
    m_sParamDisplayName = new wxStaticText( this, wxID_ANY, wxT("..."), wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_END );
	m_sParamDisplayName->SetToolTip(m_sFullDisplayName);
	fgSizer1->Add( m_sParamDisplayName, 1, wxALL|wxEXPAND, 5 );

	m_bitmap = new wxStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( m_bitmap, 0, wxALL, 5 );

	m_bPathSizer = new wxBoxSizer( wxHORIZONTAL );

	wxArrayString Choices;

    wxGISGPValueDomain* poGPValueDomain = dynamic_cast<wxGISGPValueDomain*>(m_pParam->GetDomain());
    if(poGPValueDomain)
	{
		for(size_t i = 0; i < poGPValueDomain->GetCount(); ++i)
			Choices.Add( poGPValueDomain->GetName(i));
	}
	m_choice = new wxChoice( this, ID_CHOICESTR, wxDefaultPosition, wxDefaultSize, Choices, 0 );
    if(m_pParam->GetValue().GetString().IsEmpty())
        m_choice->SetSelection( 0 );
    else
    {
		int nPos = wxNOT_FOUND;
		if(poGPValueDomain)
            nPos = poGPValueDomain->GetPosByValue(m_pParam->GetValue());

		if(nPos == wxNOT_FOUND)
			m_choice->Select(0);
		else
			m_choice->Select(nPos);
    }

	m_bPathSizer->Add( m_choice, 1, wxALL|wxEXPAND, 5 );

	fgSizer1->Add( m_bPathSizer, 0, wxALL|wxEXPAND, 5 );

	this->SetSizer( fgSizer1 );
	this->Layout();

	m_sParamDisplayName->SetLabel(m_sFullDisplayName);
}

wxGISDTChoice::~wxGISDTChoice()
{
}

bool wxGISDTChoice::Validate(void)
{
    if(m_pParam->GetHasBeenValidated())
		return m_pParam->IsValid();

	m_pParam->SetHasBeenValidated(true);

	bool bRetVal(false);

    switch(m_pParam->GetDataType())
    {
	case enumGISGPParamDTIntegerChoice:
		if(m_pParam->GetValue().GetType() == wxString(wxT("long")))
			bRetVal = true;
        break;
    case enumGISGPParamDTDoubleChoice:
		if(m_pParam->GetValue().GetType() == wxString(wxT("double")))
			bRetVal = true;
        break;
	case enumGISGPParamDTStringChoice:
	case enumGISGPParamDTStringChoiceEditable:
	default:
		if(m_pParam->GetValue().GetType() == wxString(wxT("string")))
			bRetVal = true;
    }

    m_pParam->SetValid(bRetVal);
    if ( bRetVal )
    {
        m_pParam->SetMessage(enumGISMessageOk);
    }
    else
        m_pParam->SetMessage(enumGISMessageError, _("The input data are invalid"));

    return bRetVal;

}

void wxGISDTChoice::OnChoice(wxCommandEvent& event)
{
    m_pParam->SetAltered(true);
    int nPos = m_choice->GetCurrentSelection();
    m_pParam->SetSelDomainValue(nPos);

	wxString sData = m_choice->GetStringSelection();
    bool bValid(false);
    long dVal(0);
    double dfVal(0);
    wxString sVal;

    wxGISGPValueDomain* poGPValueDomain = wxDynamicCast(m_pParam->GetDomain(), wxGISGPValueDomain);

    switch(m_pParam->GetDataType())
    {
    case enumGISGPParamDTIntegerChoice:
        if(poGPValueDomain)
        {
            dVal = poGPValueDomain->GetValue(nPos).GetLong();
            bValid = true;
        }
        else
            bValid = sData.ToLong(&dVal);
        break;
    case enumGISGPParamDTDoubleChoice:
        if(poGPValueDomain)
        {
            dfVal = poGPValueDomain->GetValue(nPos).GetDouble();
            bValid = true;
        }
        else
            bValid = sData.ToDouble(&dfVal);
        break;
    case enumGISGPParamDTStringChoice:
	case enumGISGPParamDTStringChoiceEditable:
    default:
        if(poGPValueDomain)
        {
            sVal = poGPValueDomain->GetValue(nPos).GetString();
            bValid = true;
        }
        else
        {
            sVal = sData;
            bValid = true;
        }
        break;
    }

    m_pParam->SetValid(bValid);

    if ( bValid )
    {
        switch(m_pParam->GetDataType())
        {
		case enumGISGPParamDTIntegerChoice:
            m_pParam->SetValue(wxVariant(dVal, wxT("long")));
            return;
        case enumGISGPParamDTDoubleChoice:
            m_pParam->SetValue(wxVariant(dfVal, wxT("double")));
            return;
		case enumGISGPParamDTStringChoice:
		case enumGISGPParamDTStringChoiceEditable:
		default:
            m_pParam->SetValue(wxVariant(sVal, wxT("string")));
            return;
        }
    }
}

void wxGISDTChoice::OnUpdateUI(wxUpdateUIEvent& event)
{
	Validate();
}

void wxGISDTChoice::OnParamChanged(wxGISGPParamEvent& event)
{
    if(event.GetId() == m_nParamIndex)
    {
        int nPos = m_choice->GetCurrentSelection();
        int nParamPos = m_pParam->GetSelDomainValue();
        if (nParamPos == nPos)
		    return;

        wxGISGPValueDomain* poGPValueDomain = dynamic_cast<wxGISGPValueDomain*>(m_pParam->GetDomain());
        if(poGPValueDomain)
	    {
		    //if(poGPValueDomain->GetAltered())
		    //{
 			  //  m_choice->Clear();
			   // for(size_t i = 0; i < poGPValueDomain->GetCount(); ++i)
				  //  m_choice->Append( poGPValueDomain->GetName(i));
			   // nPos = m_pParam->GetSelDomainValue();
		    //}
		    //else
		    //{
			//    int nParamPos = m_pParam->GetSelDomainValue();
			//    if(nPos == nParamPos)
			//	    return;
		    //}
                m_choice->SetSelection(nParamPos);
	    }
        Validate();
    }
}

void wxGISDTChoice::OnParamDomAddVal(wxGISGPParamEvent &event)
{
    m_choice->Append(event.GetName());
    /*if(m_choice->GetCount() == 1)
	{
        m_choice->Select(0);
		m_pParam->SetValue(event.GetParamValue());
	}*/
}

void wxGISDTChoice::OnParamDomClear(wxGISGPParamEvent &event)
{
    m_choice->Clear();
}

//-------------------------------------------------------------------------------
// Class wxGISDTFieldChoice
//-------------------------------------------------------------------------------

IMPLEMENT_CLASS(wxGISDTFieldChoice, wxGISDTChoice)

wxGISDTFieldChoice::wxGISDTFieldChoice( const wxGISGPParameterArray &Params, int nParamIndex, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxGISDTChoice( Params, nParamIndex, parent, id, pos, size, style )
{
}

wxGISDTFieldChoice::~wxGISDTFieldChoice()
{
}

void wxGISDTFieldChoice::OnParamChanged(wxGISGPParamEvent& event)
{
    if(event.GetId() == m_nParamIndex)
    {
        wxGISDTChoice::OnParamChanged(event);
    }
    else
    {
        //load fiels from vector file
        wxGISGPParameter* pParam = m_Params[event.GetId()];
		
		if(m_pParam->GetDependences().Index(pParam->GetName()) != wxNOT_FOUND)
		{			
			wxString sPath = pParam->GetValue();
			wxGxCatalogBase* pCatalog = GetGxCatalog();
			wxGxObject* pObj = pCatalog->FindGxObject(sPath);
			wxGxDataset* pGxDSet = wxDynamicCast(pObj, wxGxDataset);
			if(pGxDSet)
			{
				wxGISTable* pDSet = wxDynamicCast(pGxDSet->GetDataset(false), wxGISTable);
				wxGISPointerHolder holder(pDSet);
				if(pDSet)
				{
					OGRFeatureDefn* const pDef = pDSet->GetDefinition();
					{
						wxGISGPValueDomain* poGPValueDomain = wxDynamicCast(m_pParam->GetDomain(),wxGISGPValueDomain);
						if(poGPValueDomain)
						{
							poGPValueDomain->Clear();
						}
						else
						{
							poGPValueDomain = new wxGISGPValueDomain();
							m_pParam->SetDomain(poGPValueDomain);
						}
						for(int i = 0; i <  pDef->GetFieldCount(); ++i)
						{
							OGRFieldDefn* pFDef = pDef->GetFieldDefn(i);
							if(pFDef)
							{
								OGRFieldType eType = pFDef->GetType();
								if( (eType == OFTInteger && m_pParam->GetDataType() == enumGISGPParamDTFieldIntegerChoice) ||
									(eType == OFTReal && m_pParam->GetDataType() == enumGISGPParamDTFieldRealChoice) ||
									(eType == OFTDate && m_pParam->GetDataType() == enumGISGPParamDTFieldDateChoice) ||
									(eType == OFTString && m_pParam->GetDataType() == enumGISGPParamDTFieldStringChoice) ||
									(eType == OFTTime && m_pParam->GetDataType() == enumGISGPParamDTFieldTimeChoice) ||
									(eType == OFTDateTime && m_pParam->GetDataType() == enumGISGPParamDTFieldDateTimeChoice) ||
									(eType == OFTBinary && m_pParam->GetDataType() == enumGISGPParamDTFieldBinaryChoice) ||
									m_pParam->GetDataType() == enumGISGPParamDTFieldAnyChoice)
								{
									poGPValueDomain->AddValue(i, wxString(pFDef->GetNameRef(), wxConvUTF8));
								}
							}
						}
						if(poGPValueDomain->GetCount() > 0)
						{
							m_pParam->SetSelDomainValue(0);
							//wxString sVal = poGPValueDomain->GetValue(0).GetString();
							//if(sVal.IsEmpty())
							//{
							//    m_pParam->SetValid(false);
							//}
							//else
							//{
							//    m_pParam->SetValue(wxVariant(sVal, wxT("string")));
							//}
						}
					}
				}
			}
		}
    }
}

bool wxGISDTFieldChoice::Validate(void)
{
    if(m_pParam->GetHasBeenValidated())
		return m_pParam->IsValid();

	m_pParam->SetHasBeenValidated(true);

	bool bRetVal(false);

    switch(m_pParam->GetDataType())
    {
    case enumGISGPParamDTFieldIntegerChoice:
    case enumGISGPParamDTFieldRealChoice:
    case enumGISGPParamDTFieldDateChoice:
    case enumGISGPParamDTFieldStringChoice:
    case enumGISGPParamDTFieldTimeChoice:
    case enumGISGPParamDTFieldDateTimeChoice:
    case enumGISGPParamDTFieldBinaryChoice:
    case enumGISGPParamDTFieldAnyChoice:
	default:
        //field name or field index
		if( m_pParam->GetValue().GetType() == wxString(wxT("string")) || m_pParam->GetValue().GetType() == wxString(wxT("long")) )
			bRetVal = true;
    }

    m_pParam->SetValid(bRetVal);
    if ( bRetVal )
    {
        m_pParam->SetMessage(enumGISMessageOk);
    }
    else
        m_pParam->SetMessage(enumGISMessageError, _("The input data are invalid"));

    return bRetVal;
}

void wxGISDTFieldChoice::OnChoice(wxCommandEvent& event)
{
    m_pParam->SetAltered(true);
    int nPos = m_choice->GetCurrentSelection();
    m_pParam->SetSelDomainValue(nPos);

    //wxGISGPValueDomain* poGPValueDomain = wxDynamicCast(m_pParam->GetDomain(), wxGISGPValueDomain);

    //wxString sVal = poGPValueDomain->GetValue(nPos).GetString();
    //if(sVal.IsEmpty())
    //{
    //    m_pParam->SetValid(false);
    //}
    //else
    //{
    //    m_pParam->SetValue(wxVariant(sVal, wxT("string")));
    //}
}

/*
//------------------------------------------------------------------------------
// Class wxGISDTChoiceEditable
//------------------------------------------------------------------------------

BEGIN_EVENT_TABLE(wxGISDTChoiceEditable, wxGISDTChoice)
	EVT_BUTTON(wxID_EDIT, wxGISDTChoiceEditable::OnEdit)
END_EVENT_TABLE()

wxGISDTChoiceEditable::wxGISDTChoiceEditable( IGPParameter* pParam, IGxCatalog* pCatalog, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxGISDTChoice( pParam, pCatalog, parent, id, pos, size, style )
{
	m_bpButton = new wxBitmapButton( this, wxID_EDIT, wxBitmap(add_to_list_xpm), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	m_bPathSizer->Add( m_bpButton, 0, wxALL, 5 );
	this->Layout();
}

wxGISDTChoiceEditable::~wxGISDTChoiceEditable()
{
}

void wxGISDTChoiceEditable::OnEdit(wxCommandEvent& event)
{
	wxPGArrayStringEditorDialog dlg;
    wxGISGPStringDomain* poGPStringDomain = dynamic_cast<wxGISGPStringDomain*>(m_pParam->GetDomain());
    if(!poGPStringDomain)
		return;
	wxArrayString Choices;
	for(size_t i = 0; i < poGPStringDomain->GetCount(); ++i)
		Choices.Add( poGPStringDomain->GetName(i));


    dlg.SetDialogValue( wxVariant(Choices) );
    dlg.Create(this, _("Change value list"), _("Value list"));
    int res = dlg.ShowModal();

    if ( res == wxID_OK && dlg.IsModified() )
    {
        wxVariant value = dlg.GetDialogValue();
        if ( !value.IsNull() )
        {
			Choices = value.GetArrayString();
			poGPStringDomain->Clear();
			for(size_t i = 0; i < Choices.GetCount(); ++i)
				poGPStringDomain->AddString(Choices[i], Choices[i]);
			poGPStringDomain->SetAltered(true);

			//m_pParam->SetValue( value );
            m_pParam->SetAltered(true);
			UpdateControls();
        }
    }
}
*/
//----------------------------------------------------------------------------
// Class wxGISDTBool
//----------------------------------------------------------------------------

IMPLEMENT_CLASS(wxGISDTBool, wxGISDTBase)

BEGIN_EVENT_TABLE(wxGISDTBool, wxGISDTBase)
	EVT_CHECKBOX(ID_CHECKBOOL, wxGISDTBool::OnClick)
	EVT_SIZE(wxGISDTBool::OnSize)
	EVT_UPDATE_UI(ID_CHECKBOOL, wxGISDTBool::OnUpdateUI)
END_EVENT_TABLE()

wxGISDTBool::wxGISDTBool( const wxGISGPParameterArray &Params, int nParamIndex, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxGISDTBase( Params, nParamIndex, parent, id, pos, size, style )
{
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizer1->AddGrowableCol( 1 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_StateBitmap = new wxStaticBitmap( this, wxID_ANY, m_pParam->GetParameterType() == enumGISGPParameterTypeRequired ? m_ImageList.GetIcon(4) : wxNullBitmap , wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( m_StateBitmap, 0, wxALL, 5 );

	m_sFullDisplayName = m_pParam->GetParameterType() == enumGISGPParameterTypeOptional ? m_pParam->GetDisplayName() + wxT(" ") + _("(optional)") : m_pParam->GetDisplayName();
    m_pCheckBox = new wxCheckBox( this, ID_CHECKBOOL, m_sFullDisplayName, wxDefaultPosition, wxDefaultSize );
    m_pCheckBox->SetValue(m_pParam->GetValue());
	m_pCheckBox->SetToolTip(m_sFullDisplayName);
	fgSizer1->Add( m_pCheckBox, 1, wxALL|wxEXPAND, 5 );

	m_bitmap = new wxStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( m_bitmap, 0, wxALL, 5 );

	this->SetSizer( fgSizer1 );
	this->Layout();	
}

wxGISDTBool::~wxGISDTBool()
{
}

bool wxGISDTBool::Validate(void)
{
    if(m_pParam->GetHasBeenValidated())
		return m_pParam->IsValid();

    m_pParam->SetValid(true);
    m_pParam->SetMessage(enumGISMessageOk);
	m_pParam->SetHasBeenValidated(true);
    return true;
}

void wxGISDTBool::OnClick(wxCommandEvent& event)
{
    event.Skip();
    m_pParam->SetAltered(true);
	m_pParam->SetValue(m_pCheckBox->GetValue());
}

void wxGISDTBool::OnSize(wxSizeEvent& event)
{
    event.Skip();
	wxRect rc = GetParent()->GetClientRect();
	wxString sNewLabel = m_pCheckBox->Ellipsize(m_sFullDisplayName, wxClientDC(m_pCheckBox), wxELLIPSIZE_END, rc.GetWidth() - 80);
	m_pCheckBox->SetLabel(sNewLabel);
}

void wxGISDTBool::OnUpdateUI(wxUpdateUIEvent& event)
{
	Validate();
}

void wxGISDTBool::OnParamChanged(wxGISGPParamEvent& event)
{
    if(event.GetId() == m_nParamIndex)
    {
        m_pCheckBox->SetValue(m_pParam->GetValue().GetBool());
        Validate();
    }
}

//----------------------------------------------------------------------------
// Class wxGISDTText
//----------------------------------------------------------------------------

IMPLEMENT_CLASS(wxGISDTText, wxGISDTBase)

BEGIN_EVENT_TABLE(wxGISDTText, wxGISDTBase)
	EVT_TEXT(ID_EDITBOX, wxGISDTText::OnTextChange)
	EVT_UPDATE_UI(ID_EDITBOX, wxGISDTText::OnUpdateUI)
END_EVENT_TABLE()

wxGISDTText::wxGISDTText( const wxGISGPParameterArray &Params, int nParamIndex, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxGISDTBase( Params, nParamIndex, parent, id, pos, size, style )
{
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizer1->AddGrowableCol( 1 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_StateBitmap = new wxStaticBitmap( this, wxID_ANY, m_pParam->GetParameterType() == enumGISGPParameterTypeRequired ? m_ImageList.GetIcon(4) : wxNullBitmap , wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( m_StateBitmap, 0, wxALL, 5 );

	m_sFullDisplayName = m_pParam->GetParameterType() == enumGISGPParameterTypeOptional ? m_pParam->GetDisplayName() + wxT(" ") + _("(optional)") : m_pParam->GetDisplayName();
    m_sParamDisplayName = new wxStaticText( this, wxID_ANY, wxT("..."), wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_END );
	m_sParamDisplayName->SetToolTip(m_sFullDisplayName);
	fgSizer1->Add( m_sParamDisplayName, 1, wxALL|wxEXPAND, 5 );

	m_bitmap = new wxStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( m_bitmap, 0, wxALL, 5 );

    m_TextCtrl = new wxTextCtrl( this, ID_EDITBOX, GetParameter()->GetValue(), wxDefaultPosition, wxDefaultSize, wxTE_CHARWRAP );

    fgSizer1->Add( m_TextCtrl, 0, wxALL|wxEXPAND, 5 );

	this->SetSizer( fgSizer1 );
	this->Layout();

	m_sParamDisplayName->SetLabel(m_sFullDisplayName);

}

wxGISDTText::~wxGISDTText()
{
}

bool wxGISDTText::Validate(void)
{
    if(m_pParam->GetHasBeenValidated())
		return m_pParam->IsValid();

    if(m_TextCtrl->GetValue().IsEmpty())
    {
        m_pParam->SetAltered(false);
        if(m_pParam->GetParameterType() == enumGISGPParameterTypeOptional)
            m_pParam->SetValid(true);
        return true;
    }

    m_pParam->SetValid(true);
    m_pParam->SetMessage(enumGISMessageOk);
	m_pParam->SetHasBeenValidated(true);
    return true;
}

void wxGISDTText::OnTextChange(wxCommandEvent& event)
{
    event.Skip();
    m_pParam->SetAltered(true);
	m_pParam->SetValue(wxVariant(m_TextCtrl->GetValue(), wxT("string")));
}

void wxGISDTText::OnUpdateUI(wxUpdateUIEvent &event)
{
	Validate();
}

void wxGISDTText::OnParamChanged(wxGISGPParamEvent& event)
{
    if(event.GetId() == m_nParamIndex)
    {
		if(m_TextCtrl->GetValue() == event.GetParamValue().GetString())
			return;
        m_TextCtrl->ChangeValue( event.GetParamValue() );
        Validate();
    }
}

//-------------------------------------------------------------------------------
// Class wxGISDTSpatRef
//-------------------------------------------------------------------------------

IMPLEMENT_CLASS(wxGISDTSpatRef, wxGISDTPath)

wxGISDTSpatRef::wxGISDTSpatRef( const wxGISGPParameterArray &Params, int nParamIndex, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxGISDTPath( Params, nParamIndex, parent, id, pos, size, style )
{
	m_PathTextCtrl->SetEditable(false);
}

wxGISDTSpatRef::~wxGISDTSpatRef()
{
}

void wxGISDTSpatRef::OnOpen(wxCommandEvent& event)
{
    wxGxObjectDialog dlg(this, wxID_ANY, _("Select Spatial Reference"));
    dlg.SetAllowMultiSelect(false);
    dlg.SetAllFilters(false);
    dlg.SetOwnsFilter(true);
    dlg.SetStartingLocation(_("Coordinate Systems"));
	dlg.AddFilter(new wxGxPrjFileFilter());
    if(dlg.ShowModalOpen() == wxID_OK)
    {
		wxString sPath = dlg.GetFullName();
        wxGxCatalogBase* pCatalog = GetGxCatalog();
        wxGxObject* pGxObj = pCatalog->FindGxObject(sPath);
        wxGxPrjFileUI* pGxPrjFileUI = wxDynamicCast(pGxObj, wxGxPrjFileUI);
        if (pGxPrjFileUI)
        {
            wxGISSpatialReference SpaRef = pGxPrjFileUI->GetSpatialReference();
            wxString sDescription = SpaRef.ExportAsWKT();
            m_pParam->SetAltered(true);
            m_pParam->SetValue(wxVariant(sDescription, wxT("text")));
        
        }
    }
}

//validate
bool wxGISDTSpatRef::Validate(void)
{
    if(m_pParam->GetHasBeenValidated())
		return m_pParam->IsValid();

	m_pParam->SetHasBeenValidated(true);
    wxString sDescription = m_pParam->GetValue();
    if (sDescription.IsEmpty())
    {
        m_pParam->SetAltered(false);
        if(m_pParam->GetParameterType() != enumGISGPParameterTypeRequired)
        {
            m_pParam->SetValid(true);
            m_pParam->SetMessage(enumGISMessageNone);
            return true;
        }
        else
        {
            m_pParam->SetValid(false);
            m_pParam->SetMessage(enumGISMessageRequired, _("The value is required"));
            return false;
        }
    }

    wxGISSpatialReference oSpaRef(sDescription);
    if (oSpaRef.IsOk())
    {        
        m_pParam->SetValid(true);
        m_pParam->SetMessage(enumGISMessageOk);
        return true;
    }
    else
    {
        m_pParam->SetValid(false);
        m_pParam->SetMessage(enumGISMessageError, _("Unsupported Spatial reference"));
        return false;
    }
    return true;
}

void wxGISDTSpatRef::OnParamChanged(wxGISGPParamEvent& event)
{
    if(event.GetId() == m_nParamIndex)
    {
        wxGISSpatialReference oSpaRef(event.GetParamValue());
        if (oSpaRef.IsOk())
        {
			m_PathTextCtrl->ChangeValue( oSpaRef.GetName() );
		}
        Validate();
    }
    else
    {
        //load srs from vector file
        wxGISGPParameter* pParam = m_Params[event.GetId()];
		
		if(m_pParam->GetDependences().Index(pParam->GetName()) != wxNOT_FOUND)
		{			
			wxString sPath = pParam->GetValue();
			wxGxCatalogBase* pCatalog = GetGxCatalog();
			wxGxObject* pObj = pCatalog->FindGxObject(sPath);
			wxGxDataset* pGxDSet = wxDynamicCast(pObj, wxGxDataset);
			if(pGxDSet)
			{
                wxGISFeatureDataset* pDSet = wxDynamicCast(pGxDSet->GetDataset(false), wxGISFeatureDataset);
				wxGISPointerHolder holder(pDSet);
				if(pDSet)
				{
                    wxGISSpatialReference oSpaRef = pDSet->GetSpatialReference();
                    if (oSpaRef.IsOk())
                    {
                        m_PathTextCtrl->ChangeValue(oSpaRef.GetName());
                    }
				}
			}
		}
    }
}

bool wxGISDTSpatRef::OnDropObjects(wxCoord x, wxCoord y, const wxArrayString& GxObjects, bool bIsControlOn, bool bIsShiftOn)
{
    if (GxObjects.GetCount() > 0)
    {
		wxGxCatalogBase* pCatalog = GetGxCatalog();
		if(pCatalog)
		{
			wxGxObject* pGxObj = pCatalog->FindGxObject(GxObjects[0]);
			wxGxPrjFileUI* pGxPrjFileUI = wxDynamicCast(pGxObj, wxGxPrjFileUI);
		    if(pGxPrjFileUI)
			{
				wxGISSpatialReference oSpaRef = pGxPrjFileUI->GetSpatialReference();
				m_PathTextCtrl->ChangeValue( oSpaRef.GetName() );
                wxString sDescription = oSpaRef.ExportAsWKT();
                m_pParam->SetAltered(true);
                m_pParam->SetValue(wxVariant(sDescription, wxT("text")));
			}
		}
    }

    return true;
}

//------------------------------------------------------------------------------
// Class wxGISDTMultiParam
//------------------------------------------------------------------------------
/*
BEGIN_EVENT_TABLE(wxGISDTMultiParam, wxPanel)
	EVT_GRID_CELL_CHANGED(wxGISDTMultiParam::OnCellChange)
	EVT_UPDATE_UI(ID_PPCTRL, wxGISDTMultiParam::OnUpdateUI)
END_EVENT_TABLE()

wxGISDTMultiParam::wxGISDTMultiParam( IGPParameter* pParam, IGxCatalog* pCatalog, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxGISDTBase( pParam, parent, id, pos, size, style )
{
    m_pCatalog = pCatalog;
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizer1->AddGrowableCol( 1 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_StateBitmap = new wxStaticBitmap( this, wxID_ANY, m_pParam->GetParameterType() == enumGISGPParameterTypeRequired ? m_ImageList.GetIcon(4) : wxNullBitmap , wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( m_StateBitmap, 0, wxALL, 5 );

	m_sFullDisplayName = m_pParam->GetParameterType() == enumGISGPParameterTypeOptional ? m_pParam->GetDisplayName() + wxT(" ") + _("(optional)") : m_pParam->GetDisplayName();
    m_sParamDisplayName = new wxStaticText( this, wxID_ANY, wxT("..."), wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_END );
	m_sParamDisplayName->SetToolTip(m_sFullDisplayName);
	fgSizer1->Add( m_sParamDisplayName, 1, wxALL|wxEXPAND, 5 );

	m_bitmap = new wxStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( m_bitmap, 0, wxALL, 5 );

	wxBoxSizer* bPathSizer;
	bPathSizer = new wxBoxSizer( wxHORIZONTAL );

	wxGISGPMultiParameter* pMParam = dynamic_cast<wxGISGPMultiParameter*>(pParam);
	m_pg = new wxGrid(this, wxGISDTMultiParam::ID_PPCTRL, wxDefaultPosition, wxSize(200,200), wxBORDER_THEME | wxWANTS_CHARS);
	// Grid
	m_pg->CreateGrid(pMParam->GetRowCount(), pMParam->GetColumnCount());
	m_pg->EnableEditing( true );
	m_pg->EnableGridLines( true );
	m_pg->EnableDragGridSize( false );
	m_pg->SetMargins( 0, 0 );

	// Columns
	m_pg->EnableDragColMove( false );
	m_pg->EnableDragColSize( true );
	m_pg->SetColLabelSize( GRID_COL_SIZE );
	m_pg->SetColLabelAlignment( wxALIGN_CENTRE, wxALIGN_CENTRE );

	// Rows
	m_pg->EnableDragRowSize( true );
	m_pg->SetRowLabelSize( 4 );//GRID_ROW_SIZE );
	m_pg->SetRowLabelAlignment( wxALIGN_CENTRE, wxALIGN_CENTRE );

	// Label Appearance

	// Cell Defaults
	m_pg->SetDefaultCellAlignment( wxALIGN_LEFT, wxALIGN_TOP );
	//m_grid->SetDefaultCellBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));


	for(size_t i = 0; i < pMParam->GetColumnCount(); ++i)
	{
		m_pg->SetColLabelValue(i, pMParam->GetColumnName(i));
		m_pg->SetColSize(i, 175);
	}
	//m_pg->AutoSizeColumns();
	m_pg->AutoSizeRows();
	bPathSizer->Add( m_pg, 1, wxALL|wxEXPAND, 5 );

	fgSizer1->Add( bPathSizer, 0, wxALL|wxEXPAND, 5 );

	this->SetSizer( fgSizer1 );
	this->Layout();

	m_sParamDisplayName->SetLabel(m_sFullDisplayName);
}

wxGISDTMultiParam::~wxGISDTMultiParam()
{
}

//validate
bool wxGISDTMultiParam::Validate(void)
{
    if(m_pParam->GetHasBeenValidated())
		return m_pParam->GetIsValid();

	m_pParam->SetHasBeenValidated(true);

    if(!m_pParam->GetAltered())//TODO: Validate multiparam
        return true;

    m_pParam->SetMessage(wxGISEnumGPMessageRequired, _("The value is required"));
	if(m_pParam->GetIsValid())
	{
		m_pParam->SetMessage(wxGISEnumGPMessageOk);
		return true;
	}
	else
	{
		//TODO: Get error message and set it
		m_pParam->SetMessage(wxGISEnumGPMessageError);
		return false;
	}
	return true;
}

void wxGISDTMultiParam::UpdateControls(void)
{
	if(m_pParam->GetAltered())
		return;
	m_pParam->SetAltered(true);

	if(m_pg->GetNumberRows())
		m_pg->DeleteRows(0, m_pg->GetNumberRows());
	wxGISGPMultiParameter* pParam2 = dynamic_cast<wxGISGPMultiParameter*>(m_pParam);
	m_pg->AppendRows(pParam2->GetRowCount());
	if(pParam2)
	{
		for(size_t i = 0; i < pParam2->GetRowCount(); ++i)
		{
			for(size_t j = 0; j < pParam2->GetColumnCount(); ++j)
			{
				IGPParameter* pCellParam = pParam2->GetParameter(j, i);
				if(!pCellParam)
					continue;
				IGPDomain* pDomain = pCellParam->GetDomain();
				if(pDomain)
				{
					wxArrayString sDomainNames;
					for(size_t k = 0; k < pDomain->GetCount(); ++k)
						sDomainNames.Add(pDomain->GetName(k));
					wxGridCellChoiceEditor* pCEditor = new wxGridCellChoiceEditor(sDomainNames);
					m_pg->SetCellEditor(i, j, pCEditor);
					if(pDomain->GetCount())
						m_pg->SetCellValue(i, j, sDomainNames[pCellParam->GetSelDomainValue()]);
				}
				else
				{
					switch(pCellParam->GetDataType())
					{
					case enumGISGPParamDTBool:
						m_pg->SetColFormatBool(j);
						m_pg->SetCellValue(i, j, wxString::Format(wxT("%d"), pCellParam->GetValue().GetBool()));
						break;
					case enumGISGPParamDTInteger:
						m_pg->SetColFormatNumber(j);
						m_pg->SetCellValue(i, j, wxString::Format(wxT("%d"), pCellParam->GetValue().GetInteger()));
						break;
					case enumGISGPParamDTDouble:
						m_pg->SetColFormatFloat(j, 8, 6);
						m_pg->SetCellValue(i, j, wxString::Format(wxT("%f"), pCellParam->GetValue().GetDouble()));
						break;
					case enumGISGPParamDTString:
						m_pg->SetCellValue(i, j, pCellParam->GetValue().GetString());
						break;
						//TODO:
					case enumGISGPParamDTSpatRef:
					case enumGISGPParamDTQuery:
					case enumGISGPParamDTPath:
					case enumGISGPParamDTFolderPath:
					case enumGISGPParamDTStringList:
					case enumGISGPParamDTPathArray:
						m_pg->SetCellValue(i, j, pCellParam->GetValue().GetString());
						break;
						break;
						//end TODO
					case enumGISGPParamDTParamArray:
					case enumGISGPParamDTUnknown:
					case enumGISGPParamMax:
					default:
						break;
					}
				}
			}
		}
	}
}


void wxGISDTMultiParam::OnCellChange(wxGridEvent &event)
{
	wxGISGPMultiParameter* pParam2 = dynamic_cast<wxGISGPMultiParameter*>(m_pParam);
	wxASSERT(pParam2);

	m_pParam->SetAltered(true);
	m_pParam->SetHasBeenValidated( false );

	IGPParameter* pCellParam = pParam2->GetParameter(event.GetCol(), event.GetRow());
	if(pCellParam)
	{
        pCellParam->SetAltered(true);

		pCellParam->SetHasBeenValidated( false );
		wxString sCellValue = m_pg->GetCellValue(event.GetRow(), event.GetCol());
		IGPDomain* pDomain = pCellParam->GetDomain();
		if(pDomain)
		{
			int nPos = pDomain->GetPosByName(sCellValue);
			if(nPos != wxNOT_FOUND)
				pCellParam->SetSelDomainValue(nPos);
		}
		else
		{
			switch(pCellParam->GetDataType())
			{
			case enumGISGPParamDTBool:
				pCellParam->SetValue( wxVariant((bool)wxAtoi(sCellValue)) );
				break;
			case enumGISGPParamDTInteger:
				pCellParam->SetValue( wxVariant(wxAtoi(sCellValue)) );
				break;
			case enumGISGPParamDTDouble:
				pCellParam->SetValue( wxVariant(wxAtof(sCellValue)) );
				break;
			case enumGISGPParamDTString:
			case enumGISGPParamDTSpatRef:
			case enumGISGPParamDTPath:
			case enumGISGPParamDTFolderPath:
			case enumGISGPParamDTPathArray:
			case enumGISGPParamDTStringList:
				pCellParam->SetValue( wxVariant(sCellValue) );
				break;
			case enumGISGPParamDTUnknown:
			default:
				pCellParam->SetValue( wxVariant(sCellValue) );
				break;
			}
		}
	}
}

void wxGISDTMultiParam::UpdateValues(void)
{
	//TODO: Set values from control
}

void wxGISDTMultiParam::OnUpdateUI(wxUpdateUIEvent &event)
{
	Validate();
    SetMessage(m_pParam->GetMessageType(), m_pParam->GetMessage());
}

//------------------------------------------------------------------------------
// Class wxGISDTList
//------------------------------------------------------------------------------

BEGIN_EVENT_TABLE(wxGISDTList, wxPanel)
	EVT_BUTTON(wxID_ADD, wxGISDTList::OnAdd)
	EVT_UPDATE_UI(ID_LISTCTRL, wxGISDTList::OnUpdateUI)
	EVT_TEXT(ID_LISTCTRL, wxGISDTList::OnTextChange)
END_EVENT_TABLE()

wxGISDTList::wxGISDTList( IGPParameter* pParam, IGxCatalog* pCatalog, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxGISDTBase( pParam, parent, id, pos, size, style )
{
    m_pCatalog = pCatalog;
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizer1->AddGrowableCol( 1 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_StateBitmap = new wxStaticBitmap( this, wxID_ANY, m_pParam->GetParameterType() == enumGISGPParameterTypeRequired ? m_ImageList.GetIcon(4) : wxNullBitmap , wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( m_StateBitmap, 0, wxALL, 5 );

	m_sFullDisplayName = m_pParam->GetParameterType() == enumGISGPParameterTypeOptional ? m_pParam->GetDisplayName() + wxT(" ") + _("(optional)") : m_pParam->GetDisplayName();
    m_sParamDisplayName = new wxStaticText( this, wxID_ANY, wxT("..."), wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_END );
	m_sParamDisplayName->SetToolTip(m_sFullDisplayName);
	fgSizer1->Add( m_sParamDisplayName, 1, wxALL|wxEXPAND, 5 );

	m_bitmap = new wxStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( m_bitmap, 0, wxALL, 5 );

	wxBoxSizer* bPathSizer;
	bPathSizer = new wxBoxSizer( wxHORIZONTAL );

    m_TextCtrl = new wxTextCtrl( this, ID_LISTCTRL, GetValue(), wxDefaultPosition, wxDefaultSize, wxTE_BESTWRAP );
    //m_PathTextCtrl->SetDropTarget(new wxFileDropTarget());
	bPathSizer->Add( m_TextCtrl, 1, wxALL|wxEXPAND, 5 );

	m_bpButton = new wxBitmapButton( this, wxID_ADD, wxBitmap(add_to_list_xpm), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	bPathSizer->Add( m_bpButton, 0, wxALL, 5 );
	fgSizer1->Add( bPathSizer, 0, wxALL|wxEXPAND, 5 );

	this->SetSizer( fgSizer1 );
	this->Layout();

	m_sParamDisplayName->SetLabel(m_sFullDisplayName);
}

wxGISDTList::~wxGISDTList()
{
}

void wxGISDTList::OnAdd(wxCommandEvent& event)
{
	wxPGArrayStringEditorDialog dlg;
    dlg.SetDialogValue( m_pParam->GetValue() );
    dlg.Create(this, _("Change value list"), _("Value list"));
    int res = dlg.ShowModal();

    if ( res == wxID_OK && dlg.IsModified() )
    {
        wxVariant value = dlg.GetDialogValue();
        if ( !value.IsNull() )
        {
			m_pParam->SetValue( value );
            m_pParam->SetAltered(true);
			UpdateControls();
        }
    }
}

bool wxGISDTList::Validate(void)
{
    if(m_pParam->GetHasBeenValidated())
		return m_pParam->GetIsValid();

	m_pParam->SetHasBeenValidated(true);
    wxArrayString saValues = m_pParam->GetValue().GetArrayString();
    if(saValues.GetCount() <= 0)
	{
		if( m_pParam->GetParameterType() == enumGISGPParameterTypeRequired )
			m_pParam->SetMessage(wxGISEnumGPMessageRequired, _("The value is required"));
		else
			m_pParam->SetMessage(wxGISEnumGPMessageError, _("The input data are invalid"));

		m_pParam->SetIsValid(false);
		return false;
	}
	else
	{
		m_pParam->SetIsValid(true);
	}
    return true;
}

void wxGISDTList::UpdateControls(void)
{
	if(!m_pParam->GetAltered())
		return;
	wxString sVal  = GetValue();
	if(m_TextCtrl->GetValue().CmpNoCase(sVal) == 0)
		return;
    m_TextCtrl->ChangeValue( GetValue() );
    m_TextCtrl->SetInsertionPointEnd( );
    SetMessage(m_pParam->GetMessageType(), m_pParam->GetMessage());
}

wxString wxGISDTList::GetValue()
{
	if(!m_pParam)
		return wxEmptyString;
	wxArrayString Arr = m_pParam->GetValue().GetArrayString();
	wxString sOutput;
	for(size_t i = 0; i < Arr.GetCount(); ++i)
	{
		sOutput.Append(Arr[i].Trim(true).Trim(false));
		if(i != Arr.GetCount() - 1)
			sOutput.Append(wxT(", "));
	}
	return sOutput;
}

void wxGISDTList::UpdateValues(void)
{
	wxArrayString saValues =  wxStringTokenize(m_TextCtrl->GetValue(), wxString(wxT(",")), wxTOKEN_RET_EMPTY_ALL );//wxTOKEN_RET_EMPTY
	//for(size_t i = 0; i < saValues.GetCount(); ++i)
	//	saValues[i] = saValues[i].Trim(true).Trim(false);

	m_pParam->SetValue( wxVariant(saValues, wxT("list")) );
}

void wxGISDTList::OnUpdateUI(wxUpdateUIEvent &event)
{
	Validate();
    SetMessage(m_pParam->GetMessageType(), m_pParam->GetMessage());
}

void wxGISDTList::OnTextChange(wxCommandEvent& event)
{
    m_pParam->SetAltered(true);
	UpdateValues();
}

//------------------------------------------------------------------------------
// Class wxGISSQLQueryCtrl
//------------------------------------------------------------------------------

BEGIN_EVENT_TABLE(wxGISSQLQueryCtrl, wxPanel)
	EVT_BUTTON(wxID_OPEN, wxGISSQLQueryCtrl::OnOpen)
	EVT_UPDATE_UI(ID_LISTCTRL, wxGISSQLQueryCtrl::OnUpdateUI)
END_EVENT_TABLE()

wxGISSQLQueryCtrl::wxGISSQLQueryCtrl( IGPParameter* pParam, IGxCatalog* pCatalog, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : wxGISDTBase( pParam, parent, id, pos, size, style )
{
    m_pCatalog = pCatalog;
	wxFlexGridSizer* fgSizer1;
	fgSizer1 = new wxFlexGridSizer( 2, 2, 0, 0 );
	fgSizer1->AddGrowableCol( 1 );
	fgSizer1->SetFlexibleDirection( wxBOTH );
	fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

    m_StateBitmap = new wxStaticBitmap( this, wxID_ANY, m_pParam->GetParameterType() == enumGISGPParameterTypeRequired ? m_ImageList.GetIcon(4) : wxNullBitmap , wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( m_StateBitmap, 0, wxALL, 5 );

	m_sFullDisplayName = m_pParam->GetParameterType() == enumGISGPParameterTypeOptional ? m_pParam->GetDisplayName() + wxT(" ") + _("(optional)") : m_pParam->GetDisplayName();
    m_sParamDisplayName = new wxStaticText( this, wxID_ANY, wxT("..."), wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_END );
	m_sParamDisplayName->SetToolTip(m_sFullDisplayName);
	fgSizer1->Add( m_sParamDisplayName, 1, wxALL|wxEXPAND, 5 );

	m_bitmap = new wxStaticBitmap( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, 0 );
	fgSizer1->Add( m_bitmap, 0, wxALL, 5 );

	wxBoxSizer* bPathSizer;
	bPathSizer = new wxBoxSizer( wxHORIZONTAL );

    m_QueryTextCtrl = new wxTextCtrl( this, ID_LISTCTRL, pParam->GetValue(), wxDefaultPosition, wxDefaultSize, wxTE_BESTWRAP );
    //m_QueryTextCtrl->SetDropTarget(new wxFileDropTarget());
	bPathSizer->Add( m_QueryTextCtrl, 1, wxALL|wxEXPAND, 5 );

	m_bpButton = new wxBitmapButton( this, wxID_OPEN, wxBitmap(querysql_xpm), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
	bPathSizer->Add( m_bpButton, 0, wxALL, 5 );
	fgSizer1->Add( bPathSizer, 0, wxALL|wxEXPAND, 5 );

	this->SetSizer( fgSizer1 );
	this->Layout();

	m_sParamDisplayName->SetLabel(m_sFullDisplayName);
}

wxGISSQLQueryCtrl::~wxGISSQLQueryCtrl()
{
}

void wxGISSQLQueryCtrl::OnOpen(wxCommandEvent& event)
{
	wxGISSQLQueryDialog dlg(this);
	dlg.ShowModal();
   // wxGISGPGxObjectDomain* pDomain = dynamic_cast<wxGISGPGxObjectDomain*>(m_pParam->GetDomain());
   // wxFileName Name(m_pParam->GetValue().MakeString());


   // if(m_pParam->GetDirection() == enumGISGPParameterDirectionInput)
   // {
   //     wxGxObjectDialog dlg(this, m_pCatalog, wxID_ANY, _("Select input object"));
   //     dlg.SetAllowMultiSelect(false);
   //     dlg.SetAllFilters(false);
   //     dlg.SetOwnsFilter(false);
   //     dlg.SetName( Name.GetFullName() );
   //     if(pDomain)
   //     {
   //         for(size_t i = 0; i < pDomain->GetCount(); ++i)
   //         {
			//	if(i == m_pParam->GetSelDomainValue())
   //                 dlg.AddFilter(pDomain->GetFilter(i), true);
   //             else
   //                 dlg.AddFilter(pDomain->GetFilter(i), false);
   //         }
   //     }
   //     dlg.SetOverwritePrompt(false);
   //     if(dlg.ShowModalOpen() == wxID_OK)
   //     {
   //         wxString sPath = dlg.GetFullPath();
   //         //sPath.Replace(wxT("\\\\"), wxT("\\"));
   //         //m_PathTextCtrl->ChangeValue( sPath );
   //         m_pParam->SetAltered(true);
			//m_pParam->SetSelDomainValue(dlg.GetCurrentFilterId());
   //         m_pParam->SetValue(wxVariant(sPath, wxT("path")));
   //     }
   // }
   // else
   // {
   //     wxGxObjectDialog dlg(this, m_pCatalog, wxID_ANY, _("Select output object"));
   //     dlg.SetAllowMultiSelect(false);
   //     dlg.SetAllFilters(false);
   //     dlg.SetOwnsFilter(false);
   //     dlg.SetName( Name.GetFullName() );
   //     if(pDomain)
   //     {
   //         for(size_t i = 0; i < pDomain->GetCount(); ++i)
   //         {
			//	if(i == m_pParam->GetSelDomainValue())
   //                 dlg.AddFilter(pDomain->GetFilter(i), true);
   //             else
   //                 dlg.AddFilter(pDomain->GetFilter(i), false);
   //         }
   //     }
   //     dlg.SetOverwritePrompt(false);
   //     if(dlg.ShowModalSave() == wxID_OK)
   //     {
   //         wxString sPath = dlg.GetFullPath();
   //         m_pParam->SetAltered(true);
			//m_pParam->SetSelDomainValue(dlg.GetCurrentFilterId());
   //         m_pParam->SetValue(wxVariant(sPath, wxT("path")));
   //     }
   // }
}

//validate
bool wxGISSQLQueryCtrl::Validate(void)
{
    //if(m_pParam->GetHasBeenValidated())
    //    return true;

    //wxString sPath = m_pParam->GetValue();
    //if(sPath.IsEmpty())
    //{
    //    m_pParam->SetAltered(false);
    //    if(m_pParam->GetParameterType() != enumGISGPParameterTypeRequired)
    //    {
    //        m_pParam->SetIsValid(true);
    //        m_pParam->SetMessage(wxGISEnumGPMessageNone);
    //        return true;
    //    }
    //    else
    //    {
    //        m_pParam->SetIsValid(false);
    //        m_pParam->SetMessage(wxGISEnumGPMessageRequired, _("The value is required"));
    //        return false;
    //    }
    //}
    //if(m_pCatalog)
    //{
    //    IGxObjectContainer* pGxContainer = dynamic_cast<IGxObjectContainer*>(m_pCatalog);
    //    IGxObject* pGxObj = pGxContainer->SearchChild(sPath);
    //    if(pGxObj)
    //    {
    //       if(m_pParam->GetDirection() == enumGISGPParameterDirectionInput)
    //       {
    //           if(m_pParam->GetMessageType() == wxGISEnumGPMessageError)
    //               return false;
    //           m_pParam->SetIsValid(true);
    //           m_pParam->SetMessage(wxGISEnumGPMessageOk);
    //       }
    //       else
    //       {
    //           if(m_pParam->GetMessageType() == wxGISEnumGPMessageError)
    //               return false;
    //           m_pParam->SetIsValid(true);
    //           m_pParam->SetMessage(wxGISEnumGPMessageWarning, _("The output object exists and will be overwritten!"));
    //       }
    //       return true;
    //    }
    //    else
    //    {
    //       if(m_pParam->GetDirection() == enumGISGPParameterDirectionInput)
    //       {
    //            m_pParam->SetIsValid(false);
    //            m_pParam->SetMessage(wxGISEnumGPMessageError, _("The input object doesn't exist"));
    //            return false;
    //       }
    //       else
    //       {
    //           if(m_pParam->GetMessageType() == wxGISEnumGPMessageError)
    //               return false;

    //           m_pParam->SetIsValid(true);
    //           m_pParam->SetMessage(wxGISEnumGPMessageOk);
    //           return true;
    //       }
    //    }
    //}
    return true;
}

void wxGISSQLQueryCtrl::UpdateControls(void)
{
 //   m_PathTextCtrl->ChangeValue( m_pParam->GetValue() );
 //   SetMessage(m_pParam->GetMessageType(), m_pParam->GetMessage());

 //   wxGISGPGxObjectDomain* poDomain = dynamic_cast<wxGISGPGxObjectDomain*>(m_pParam->GetDomain());
	//if(poDomain && poDomain->GetCount() > 0)
	//{
	//	IGxObjectFilter* poFilter = poDomain->GetFilter(m_pParam->GetSelDomainValue());
	//	wxFileName oName(m_pParam->GetValue());
	//	if(!oName.IsOk())
	//		return;

	//	for(size_t i = 0; i < poDomain->GetCount(); ++i)
	//	{
	//		poFilter = poDomain->GetFilter(i);
	//		if(poFilter)
	//		{
	//			if(oName.GetExt().CmpNoCase(poFilter->GetExt()) == 0 || poFilter->GetExt() == wxEmptyString)
	//			{
	//				m_pParam->SetSelDomainValue(i);
	//				break;
	//			}
	//		}
	//	}
 //   //Validate();
	//}
}

void wxGISSQLQueryCtrl::UpdateValues(void)
{
	//TODO: Set values from control
}

void wxGISSQLQueryCtrl::OnUpdateUI(wxUpdateUIEvent &event)
{
	UpdateValues();
	Validate();
}
*/
