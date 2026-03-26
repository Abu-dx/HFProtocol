// DirTreeCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "shlobj.h"
#include "DirTreeCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDirTreeCtrl::CDirTreeCtrl()
{
}

CDirTreeCtrl::~CDirTreeCtrl()
{
}


BEGIN_MESSAGE_MAP(CDirTreeCtrl, CTreeCtrl)
	//{{AFX_MSG_MAP(CDirTreeCtrl)
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDED, OnItemexpanded)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDirTreeCtrl message handlers

void CDirTreeCtrl::Initialize(CString path,bool clear)
{
	LPITEMIDLIST itemlist;
	char buf[300];
	int desktopIcon;
	int mycomputerIcon;
    int ret;

    if (clear)
	{
		m_img.DeleteImageList();
	    DeleteAllItems();
	}
	SetImageList (NULL,0);

//	SHGetSpecialFolderLocation (this->m_hWnd, CSIDL_DESKTOP, &itemlist);
//	SHGetPathFromIDList(itemlist, buf ); 
	m_desktop_path = path;

	m_img.Create( GetSystemMetrics(SM_CXSMICON), 
		          GetSystemMetrics(SM_CYSMICON), 
				  ILC_COLOR24, 50, 50);

	m_img.SetBkColor( GetSysColor(COLOR_WINDOW) );

    CBitmap bmp;
    CBitmap *old;
    CBitmap *newbmp;
    CDC dc;

    dc.CreateCompatibleDC(NULL);
    bmp.CreateBitmap (32,32,1,24,NULL);
    old=dc.SelectObject (&bmp);
    dc.FillSolidRect(0,0,31,31,GetSysColor(COLOR_WINDOW));
    newbmp=dc.SelectObject (old);
    dc.DeleteDC ();
    ret=m_img.Add(newbmp,COLORREF(0x00));


	/*desktopIcon=    m_img.Add(ExtractIcon( AfxGetApp()->m_hInstance, "shell32.dll", 34));
	mycomputerIcon= m_img.Add(ExtractIcon( AfxGetApp()->m_hInstance, "shell32.dll", 15));*/

	SHFILEINFO shinfo, shinfo_sel;
	SHGetFileInfo( "C:\\Windows", NULL, 
		&shinfo, 
		sizeof(shinfo), 
		SHGFI_DISPLAYNAME | SHGFI_ICON | SHGFI_SMALLICON);

	SHGetFileInfo( "C:\\Windows", NULL, 
		&shinfo_sel, 
		sizeof(shinfo_sel), 
		SHGFI_DISPLAYNAME |  SHGFI_ICON | SHGFI_OPENICON | SHGFI_SMALLICON);

	desktopIcon=m_img.Add(shinfo.hIcon);
//	int iconsel = iconEx2!=-1 ? iconEx2 : m_img.Add(shinfo_sel.hIcon);


//	m_drives_root=AddItem_Core("ÎÒµÄµçÄÔ", TVI_ROOT, true, mycomputerIcon, mycomputerIcon);
	m_desktop_root=AddItem(m_desktop_path, ((HTREEITEM)0xFFFF0000), true, desktopIcon, desktopIcon);
}

HTREEITEM CDirTreeCtrl::AddItem(const char * path, HTREEITEM parent, bool isDir, int iconEx, int iconEx2)
{
	
    SHFILEINFO shinfo, shinfo_sel;

	SHGetFileInfo( path, NULL, 
				   &shinfo, 
				   sizeof(shinfo), 
				   SHGFI_DISPLAYNAME | SHGFI_ICON | SHGFI_SMALLICON);

	SHGetFileInfo( path, NULL, 
				   &shinfo_sel, 
				   sizeof(shinfo_sel), 
				   SHGFI_DISPLAYNAME |  SHGFI_ICON | SHGFI_OPENICON | SHGFI_SMALLICON);

	int icon    = iconEx!=-1 ? iconEx : m_img.Add(shinfo.hIcon);
    int iconsel = iconEx2!=-1 ? iconEx2 : m_img.Add(shinfo_sel.hIcon);

	SetImageList (&m_img,LVSIL_NORMAL);

	return AddItem_Core ( shinfo.szDisplayName, parent, isDir, icon, iconsel);
}

void CDirTreeCtrl::OnItemexpanded(NMHDR* pNMHDR, LRESULT* pResult) 
{
	
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	TV_ITEM itm= pNMTreeView->itemNew;


	if (itm.state & TVIS_EXPANDED)
	{

		//AfxMessageBox();
		ExpandBranch(itm.hItem);
	}
	else
	{
		DeleteAllChild(itm.hItem);
	}
	
	*pResult = 0;
}

void CDirTreeCtrl::DeleteAllChild(HTREEITEM itm, bool isEmpty)
{
	HTREEITEM child;

	child= GetChildItem(itm);

	while (child)
	{
		int img1, img2;
		
        if (GetChildItem(child))
            DeleteAllChild(GetChildItem(child), isEmpty);

        GetItemImage(child,img1,img2);

        if (img2!=0 && img2!=img1) 
		    {
            m_img.Remove(img2);
            
      //      RefreshTreeImages(m_drives_root, img2);
            RefreshTreeImages(m_desktop_root, img2);
            }

		if (img1!= 0)
            {
            m_img.Remove(img1);

        //    RefreshTreeImages(m_drives_root, img1);
            RefreshTreeImages(m_desktop_root, img1);
            }
		
        DeleteItem (child);

        SetImageList(&m_img,TVSIL_NORMAL);

		child= GetChildItem(itm);
	}

	if (isEmpty)
		InsertItem( "",itm);; 
}


void CDirTreeCtrl::ExpandBranch(HTREEITEM parent)
{
    bool bOne=false;
	CString fi;
	if(parent==m_drives_root)
	{
		char *ptr;

    	DeleteAllChild(parent,false);
		GetLogicalDriveStrings(500,m_drives_name);

		ptr=m_drives_name;

		while (*ptr)
		{
            bOne=true;
			AddItem(ptr, m_drives_root, true);
			ptr+=strlen(ptr)+1;
		}
	}
	else
	{
        CString fullpath,pathWildcard;
        fullpath=GetPathFromHere(fullpath, parent);
//		AfxMessageBox(fullpath);
        DeleteAllChild(parent,false);
        pathWildcard=fullpath+"*.*";

        CFileFind finder;
        BOOL bWorking = finder.FindFile(pathWildcard);
         while (bWorking)
            {
            bWorking = finder.FindNextFile();
			fi = finder.GetFileName();
            if ( finder.IsDirectory() && !finder.IsDots() )
                {
                bOne=true;
                HTREEITEM itm=AddItem (fullpath+finder.GetFileName(), parent );
                InsertItem("",itm);
                }
            }
        
		pathWildcard=fullpath+"*.wav";
        bWorking = finder.FindFile(pathWildcard);
		
        while (bWorking)
            {	
            bWorking = finder.FindNextFile();
			fi = finder.GetFileName();
            if ( !finder.IsDirectory() && !finder.IsDots() )
                {
                bOne=true;
                AddItem (fullpath+finder.GetFileName(), parent );
                }
            }
		pathWildcard=fullpath+"*.dat";
		bWorking = finder.FindFile(pathWildcard);
		while (bWorking)
		{
			bWorking = finder.FindNextFile();
			fi = finder.GetFileName();
			if ( !finder.IsDirectory() && !finder.IsDots() )
			{
				bOne=true;
				AddItem (fullpath+finder.GetFileName(), parent );
			}
		}

	}
    
    if (!bOne) InsertItem( "",parent);
}

HTREEITEM CDirTreeCtrl::AddItem_Core(const char * name, HTREEITEM parent, bool isDir, int icon,int iconOpen)
{
	HTREEITEM hitm;

	hitm=InsertItem( name, icon, iconOpen, parent);
	if (isDir) InsertItem( "",hitm);

	return hitm;
}

CString CDirTreeCtrl::GetPathFromHere(CString s, HTREEITEM itm)
{
    HTREEITEM parent=GetParentItem(itm);
    HTREEITEM itmChild=GetChildItem(itm);

    if (itm==m_drives_root) return "";
        
//	if (parent==m_drives_root)
//	{
////		AfxMessageBox("aa");
//		HTREEITEM child;
//
//		char *ptr=m_drives_name;
//		child=GetChildItem(parent);
//		
//		while (child)
//		{
//			if (child==itm)
//			{
//				s=CString(ptr)+s;
//				return s;
//			}
//			ptr+=strlen(ptr)+1;
//			child=GetNextItem(child, TVGN_NEXT);
//		}
//	}
//	else 
	if (itm==m_desktop_root)
	{
		
		s=m_desktop_path+'\\'+s;
		return s;
	}
    else
    {
	//	AfxMessageBox("hh");
        if (GetItemText(itm)!= CString(""))
            s=GetItemText(itm)+'\\'+s;

        s=GetPathFromHere(s,parent);
    }

    if (!itmChild && s.GetLength()>0)
        s=s.Left (s.GetLength()-1);

	return s;
}

void CDirTreeCtrl::RefreshTreeImages(HTREEITEM item,int img)
{
    int iStandard, iOpen;
    HTREEITEM itmCurrent;
       
    itmCurrent=GetChildItem(item);
    
    while (itmCurrent)
        {
        if (GetItemImage(itmCurrent,iStandard,iOpen))
            {
            if (iStandard>img) iStandard--;
            if (iOpen>img) iOpen--;

            SetItemImage(itmCurrent,iStandard, iOpen);
            }

        if (ItemHasChildren( itmCurrent)!=0)
            {
            RefreshTreeImages(itmCurrent, img);
            }

        itmCurrent=GetNextSiblingItem(itmCurrent);
        }
}

bool CDirTreeCtrl::MatchExtension(CString file)
{
    return file.Right(4)==CString(".exe");
}

CString CDirTreeCtrl::GetFullPath(HTREEITEM item)
{
    if (GetItemText(item)==CString("")) return "";
    else return GetPathFromHere("",item);
}

bool CDirTreeCtrl::SetPath(CString path_in)
{
    char path[MAX_PATH];
    char *delimiters="\\";
    char *token;

    char *pathStart=path;

    HTREEITEM current=m_drives_root;

    strcpy_s(path,path_in);

    Expand(current,TVE_EXPAND );
    token=strtok (path,delimiters);
    while (token!=NULL)
        {
        current=Dir2Item(current, token);
        if (!current)
            {
            return false;
            }
        
        Expand(current,TVE_EXPAND );
        token=strtok(NULL,delimiters);
        }

    SelectItem(current);
    return true;
}
void CDirTreeCtrl::Setdesktop_path (CString path)
{
	m_desktop_path = path;
	

}
HTREEITEM CDirTreeCtrl::Dir2Item(HTREEITEM current, char *dir)
{
    if (current==m_drives_root)
        {
        HTREEITEM child;
        CString szDir(dir);

        szDir+="\\";
        szDir.MakeUpper();

		char *ptr=m_drives_name;
		child=GetChildItem(current);
		
		while (child)
		    {
			if (szDir==CString(ptr))
		    	{
				return child;
			    }
			ptr+=strlen(ptr)+1;
			child=GetNextItem(child, TVGN_NEXT);
		    }
        }
    else
        {
        HTREEITEM child;
        CString szChildText;
        CString szDir(dir);

        szDir.MakeUpper();

		child=GetChildItem(current);
		while (child)
		    {
            szChildText=GetItemText(child);
            szChildText.MakeUpper();

			if ( szChildText==szDir)
		    	{
				return child;
			    }
			child=GetNextItem(child, TVGN_NEXT);
		    }
        }

    return NULL;
}

int CDirTreeCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CTreeCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;
	
//	Initialize();
	
	return 0;
}
