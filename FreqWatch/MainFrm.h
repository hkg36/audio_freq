// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "DIBBitmap.h"
class CMainFrame : 
	public CFrameWindowImpl<CMainFrame>, 
	public CUpdateUI<CMainFrame>,
	public CMessageFilter, public CIdleHandler
{
public:
	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

	CFreqWatchView m_view;
	CTrackBarCtrl m_trackBar;

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		if(CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
			return TRUE;

		return m_view.PreTranslateMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		UIUpdateToolBar();
		return FALSE;
	}

	BEGIN_UPDATE_UI_MAP(CMainFrame)
		UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP_EX(CMainFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MSG_WM_SIZE(OnSize)
		MSG_WM_HSCROLL(OnHScroll)
		MESSAGE_HANDLER(WM_CLIENTMOUSEMOVE,OnClientMouseMove)
		COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
		COMMAND_ID_HANDLER(ID_FILE_NEW, OnFileNew)
		COMMAND_ID_HANDLER(ID_VIEW_TOOLBAR, OnViewToolBar)
		COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		//CreateSimpleToolBar();

		m_view.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
		m_view.image=&memimage;
		m_view.freqinfos=&freqinfos;
		m_trackBar.Create(m_hWnd,rcDefault,NULL,WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,0,5);
		m_trackBar.SetRange(0,100);

		//UIAddToolBar(m_hWndToolBar);
		//UISetCheck(ID_VIEW_TOOLBAR, 1);

		// register object for message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);

		return 0;
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		// unregister message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->RemoveMessageFilter(this);
		pLoop->RemoveIdleHandler(this);

		bHandled = FALSE;
		return 1;
	}

	LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		PostMessage(WM_CLOSE);
		return 0;
	}
	std::vector<std::vector<double>> dataline;
	CDIBBitmap memimage;
	
	double maxStrong;
	LRESULT OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CFileDialog openfile(TRUE);
		if(IDOK==openfile.DoModal())
		{
			FILE* fp=NULL;
			if(0==_tfopen_s(&fp,openfile.m_szFileName,_T("rb")))
			{
				dataline.clear();
				maxStrong=0;
				while(true)
				{
					std::vector<double> buffer(SampleCount/2);
					
					size_t pos=0;
					while(true)
					{
						size_t dcount=fread(&buffer[pos],sizeof(double),SampleCount/2-pos,fp);
						if(dcount==0)
							break;
						pos+=dcount;
						if(pos==SampleCount/2)
							break;
					}
					if(pos<SampleCount/2)
						break;
					else
					{
						for(size_t i=0;i<SampleCount/2;i++)
						{
							maxStrong=max(maxStrong,buffer[i]);
						}
						dataline.push_back(std::move(buffer));
					}
				}
				fclose(fp);

				m_trackBar.SetRangeMax(100);
				m_trackBar.SetPos(50);
				BuildData();
				BuildImage();
			}
		}
		return 0;
	}
	std::vector<std::vector<double>> darklines;
	std::vector<FreqInfo> freqinfos;
	void BuildData()
	{
		darklines.clear();
		for(int i=0;i!=dataline.size();i++)
		{
			std::vector<double> line(SampleCount/2);
			for(size_t j=0;j<SampleCount/2;j++)
			{
				const int checkR=1;
				if(i>=checkR && i<dataline.size()-checkR && j>=checkR && j<SampleCount/2-checkR)
				{
					double maxv=0,minv=1e10;
					const double core1[3][3]={
						{-1,0,1},
						{-2,0,2},
						{-1,0,1}
					};
					const double core2[3][3]={
						{1,2,1},
						{0,0,0},
						{-1,-2,-1}
					};
					double gx=0;
					double gy=0;
					for(int testi=-checkR;testi<checkR;testi++)
					{
						for(int testj=-checkR;testj<checkR;testj++)
						{
							double v=dataline[i+testi][j+testj];
							gx+=v*core1[testi+1][testj+1];
							gy+=v*core2[testi+1][testj+1];
						}
					}
					double gpoint=sqrt(gx*gx+gy*gy);
					line[j]=gpoint;
				}
			}
			darklines.push_back(std::move(line));
		}
		double darkmax=0,darkmin=1e20;
		for(int i=1;i!=darklines.size()-1;i++)
		{
			for(size_t j=1;j<SampleCount/2-1;j++)
			{
				double v=darklines[i][j];
				darkmax=max(darkmax,v);
				darkmin=min(darkmin,v);
			}
		}
		double darkspan=darkmax-darkmin;
		for(int i=1;i!=darklines.size()-1;i++)
		{
			for(size_t j=1;j<SampleCount/2-1;j++)
			{
				double &v=darklines[i][j];
				v=(v-darkmin)/darkspan;
			}
		}
		
		freqinfos.clear();
		const int area=5;
		for(int i=area;i!=darklines.size()-area;i++)
		{
			for(size_t j=area;j<SampleCount/2-area;j++)
			{
				double strong=darklines[i][j];
				if(strong>0.13)
				{
					double maxstrong=strong;
					for(int x=-area;x<=area;x++)
					{
						for(int y=-area;y<=area;y++)
						{
							maxstrong=max(maxstrong,darklines[i+x][j+y]);
							if(maxstrong>strong)
								goto LOOPEND;
							/*if(x!=0 && y!=0)
							{
								double other=darklines[i+x][j+y];
								if(other>strong)
								{
									goto NEXT;
								}
							}*/
						}
					}
					LOOPEND:
					if(maxstrong==strong)
					{
						FreqInfo info;
						info.freq=j;
						info.time=i;
						info.strong=strong;
						freqinfos.push_back(info);
					}
				}
				//NEXT:;
			}
		}
	}
	
	void BuildImage()
	{
		/*if(memimage.IsNull()==FALSE)
			memimage.Destroy();
		
		BOOL res=memimage.CreateEx(SampleCount/2,dataline.size(),24,BI_RGB);
		CDCHandle dch=memimage.GetDC();
		CBrush srcbrush=dch.SelectBrush(NULL);
		CPen pen;
		CPen srcpen=pen.CreatePen(PS_SOLID,2,RGB(0,0,0));
		dch.SelectPen(pen);
		dch.FillSolidRect(0,0,memimage.GetWidth(),memimage.GetHeight(),RGB(255,255,255));*/
		memimage.SetBitmapSize(SampleCount/2,dataline.size());
		for(int i=0;i!=darklines.size();i++)
		{
			for(size_t j=0;j<SampleCount/2;j++)
			{
				const double back[]={255,255,255};

				double strong=darklines[i][j];
				strong=min(strong/((double)m_trackBar.GetPos()/100),1);
				memimage.SetPixel(j,i,strong*255+(1-strong)*back[0],(1-strong)*back[1],(1-strong)*back[2]);
			}
		}

		/*for(auto i=freqinfos.begin();i!=freqinfos.end();i++)
		{
			dch.MoveTo(i->freq-3,i->time-3);
			dch.LineTo(i->freq+3,i->time+3);
			dch.MoveTo(i->freq-3,i->time+3);
			dch.LineTo(i->freq+3,i->time-3);
		}

		CPen linepen;
		linepen.CreatePen(PS_SOLID,1,RGB(0,0,255));
		dch.SelectPen(linepen);

		for(size_t j=20;j<SampleCount/2;j+=40)
		{
			dch.MoveTo(j,0);
			dch.LineTo(j,dataline.size()-1);
		}

		memimage.ReleaseDC();
		*/
		m_view.Invalidate();
		m_view.SetScrollSize(SampleCount/2,dataline.size());
	}
	void OnSize(UINT nType, CSize size)
	{
		m_view.MoveWindow(0,0,size.cx,size.cy-30);
		m_trackBar.MoveWindow(0,size.cy-30,size.cx,30);
	}
	void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar pScrollBar)
	{
		if(pScrollBar.m_hWnd==m_trackBar.m_hWnd)
		{
			if(nSBCode==TB_ENDTRACK)
			{
				BuildImage();
			}
		}
	}
	LRESULT OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		BOOL bVisible = !::IsWindowVisible(m_hWndToolBar);
		::ShowWindow(m_hWndToolBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
		UISetCheck(ID_VIEW_TOOLBAR, bVisible);
		UpdateLayout();
		return 0;
	}

	LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CAboutDlg dlg;
		dlg.DoModal();
		return 0;
	}
	LRESULT OnClientMouseMove(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		CPoint p;
		p.x=wParam;
		p.y=lParam;
		if(p.x<SampleCount/2 && p.y<darklines.size())
		{
			CAtlString str;
			str.Format(_T("%d,%d==>%.5f"),p.x,p.y,darklines[p.y][p.x]);
			SetWindowText(str);
		}
		return S_OK;
	}
};
