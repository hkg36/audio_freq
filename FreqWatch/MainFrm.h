// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "DIBBitmap.h"
#include "music_reader.h"
#include <Mfapi.h>
#include "UploadFreqData.h"
#include "SearchBySite.h"
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
		COMMAND_ID_HANDLER(ID_FILE_SAVE,OnFileSave)
		COMMAND_ID_HANDLER(ID_FILE_SAVE_AS,OnUploadData)
		COMMAND_ID_HANDLER(ID_FILE_OPEN,OnFileOpen)
		COMMAND_ID_HANDLER(ID_FILE_SEARCH_VAR_SITE,OnSearchVarSite)
		COMMAND_ID_HANDLER(ID_VIEW_TOOLBAR, OnViewToolBar)
		COMMAND_ID_HANDLER(ID_FILE_RECORD,OnFileRecord)
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
	CAtlString openFileName;
	LRESULT OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CFileDialog openfile(TRUE);
		if(IDOK!=openfile.DoModal())
			return S_FALSE;

		if(S_OK != MFStartup(MF_VERSION))
		{
			return 0;
		}
		openFileName=openfile.m_szFileName;
		openFileName=openFileName.Right(openFileName.GetLength()-openFileName.ReverseFind('\\')-1);
		openFileName=openFileName.Left(openFileName.Find('.'));
		dataline= ReadMusicFrequencyData(openfile.m_szFileName);
		MFShutdown();
		m_trackBar.SetRangeMax(100);
		m_trackBar.SetPos(50);
		BuildData();
		BuildImage();
		return 0;
	}
	LRESULT OnFileSave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CAboutDlg dlg;
		dlg.filename=openFileName;
		if(dlg.DoModal()!=IDOK)
			return S_OK;
		dlg.filename.Trim(L"\t\n ");
		if(dlg.filename.IsEmpty())
		{
			MessageBox(L"歌曲名是空的,请检查.");
			return S_OK;
		}
		CSqlite db;
		db.Open(L"D:\\freq_info.data.db");
		CSqliteStmt insertFileName=db.Prepare(L"insert into songlist(name) values(?1)");
		CSqliteStmt lastFileId=db.Prepare(L"select last_insert_rowid() from songlist");
		CSqliteStmt insertAnchor = db.Prepare(L"insert into Anchor_freq_index(freq,time,song_id) values(?1,?2,?3)");
		CSqliteStmt lastAnchorID = db.Prepare(L"select last_insert_rowid() from Anchor_freq_index");
		CSqliteStmt insertCheck=db.Prepare(L"insert into Check_freq_index(Anchor_id,freq,time_offset) values(?1,?2,?3)");

		int song_id=0;
		insertFileName.Bind(1,dlg.filename);
		if(SQLITE_DONE==insertFileName.Step())
		{
			if(SQLITE_ROW==lastFileId.Step())
			{
				song_id=lastFileId.GetInt(0);
			}
			lastFileId.Reset();
		}
		insertFileName.Reset();

		if(song_id>0)
		{
			db.Execute(L"begin transaction");
			std::vector<FreqInfo> freqinfos_use;
			for(auto i=freqinfos.begin();i<freqinfos.end();i++)
			{
				if(i->freq>20 && i->freq<400)
				{
					freqinfos_use.push_back(*i);
				}
			}
			int ptcount=0;
			for(auto i=freqinfos_use.begin();i<freqinfos_use.end();i++)
			{
				std::vector<FreqInfo> anchor_sub;
				for(auto j=i+1;j<freqinfos_use.end();j++)
				{
					if(j->time>i->time+45)
						break;
					if(j->freq>i->freq-30 && j->freq<i->freq+30 &&
						j->time>i->time+5)
					{
						anchor_sub.push_back(*j);
					}
				}
				if(anchor_sub.size()>3)
				{
					insertAnchor.Bind(1,i->freq);
					insertAnchor.Bind(2,i->time);
					insertAnchor.Bind(3,song_id);
					if(SQLITE_DONE==insertAnchor.Step())
					{
						insertAnchor.Reset();
						int insert_id=0;
						if(SQLITE_ROW==lastAnchorID.Step())
						{
							insert_id=lastAnchorID.GetInt(0);
							lastAnchorID.Reset();
						}
						if(insert_id)
						{
							for(auto j=anchor_sub.begin();j!=anchor_sub.end();j++)
							{
								insertCheck.Bind(1,insert_id);
								insertCheck.Bind(2,j->freq);
								insertCheck.Bind(3,j->time-i->time);
								insertCheck.Step();
								insertCheck.Reset();
							}
						}
					}
				}
			}
			db.Execute(L"commit transaction");
		}
		insertFileName.Close();
		lastFileId.Close();
		insertAnchor.Close();
		lastAnchorID.Close();
		insertCheck.Close();
		db.Close();
		return S_OK;
	}
	LRESULT OnFileOpen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		struct FreqPoint
		{
			int id;
			int freq;
			int time;
			int song_id;
		};
		struct FreqPointPicked:public FreqPoint
		{
			int sample_time;
			int match_count;
			int start_time_point;
		};

		CSqlite db;
		db.Open(L"D:\\freq_info.data.db",SQLITE_OPEN_READONLY);
		CSqliteStmt findAnchor = db.Prepare(L"select id,freq,time,song_id from Anchor_freq_index where freq=?1");
		CSqliteStmt findCheckPoint =db.Prepare(L"select count(*) from Check_freq_index where Anchor_id=?1 and freq>=?2 and freq<=?3 and time_offset>=?4 and time_offset<=?5");

		std::vector<FreqInfo> freqinfos_use;
		for(auto i=freqinfos.begin();i<freqinfos.end();i++)
		{
			if(i->freq>20 && i->freq<400)
			{
				freqinfos_use.push_back(*i);
			}
		}
		int ptcount=0;
		size_t maybecount=0;
		vector<FreqPointPicked> checkres;
		for(auto i=freqinfos_use.begin();i<freqinfos_use.end();i++)
		{
			vector<FreqPoint> maybeid;
			findAnchor.Bind(1,i->freq);
			while(SQLITE_ROW==findAnchor.Step())
			{
				FreqPoint fpoint;
				fpoint.id=findAnchor.GetInt(0);
				fpoint.freq=findAnchor.GetInt(1);
				fpoint.time=findAnchor.GetInt(2);
				fpoint.song_id=findAnchor.GetInt(3);
				maybeid.push_back(fpoint);
			}
			findAnchor.Reset();
			
			if(maybeid.empty())
				continue;
			maybecount+=maybeid.size();
			for(auto maybe=maybeid.begin();maybe!=maybeid.end();maybe++)
			{
				int match_count=0;
				for(auto j=i+1;j<freqinfos_use.end();j++)
				{
					if(j->time>i->time+45)
						break;
					if(j->freq>i->freq-30 && j->freq<i->freq+30 &&
						j->time>i->time+5)
					{
						findCheckPoint.Bind(1,maybe->id);
						findCheckPoint.Bind(2,j->freq-1);
						findCheckPoint.Bind(3,j->freq+1);
						findCheckPoint.Bind(4,j->time-i->time-1);
						findCheckPoint.Bind(5,j->time-i->time+1);
						if(SQLITE_ROW==findCheckPoint.Step())
						{
							int count=findCheckPoint.GetInt(0);
							if(count)
							{
								match_count++;
							}
						}
						findCheckPoint.Reset();
					}
				}
				if(match_count>=2)
				{
					FreqPointPicked picked;
					picked.id=maybe->id;
					picked.freq=maybe->freq;
					picked.time=maybe->time;
					picked.song_id=maybe->song_id;
					picked.match_count=match_count;
					picked.sample_time=i->time;
					picked.start_time_point=picked.time-picked.sample_time;
					checkres.push_back(picked);
				}
			}
		}

		findAnchor.Close();
		findCheckPoint.Close();
		db.Close();

		CAtlString resinfo;
		resinfo.AppendFormat(_T("all Anchor checked:%u\n"),maybecount);
		struct MatchInfo
		{
			int count;
			int starttimeMaxCount;
			std::map<int,int> timematch;
			MatchInfo()
			{
				count=0;
				starttimeMaxCount=0;
			}
		};
		std::map<int,MatchInfo> matchcountor;
		for(auto i=checkres.begin();i!=checkres.end();i++)
		{
			auto& countor=matchcountor[i->song_id];
			countor.count++;
			countor.timematch[i->start_time_point]++;
		}
		for(auto countor=matchcountor.begin();countor!=matchcountor.end();countor++)
		{
			int maxpos=0;
			int maxcount=0;
			for(auto j=countor->second.timematch.begin();
				j!=countor->second.timematch.end();
				j++)
			{
				if(j->second>maxcount)
				{
					maxcount=j->second;
					maxpos=j->first;
				}
			}
			countor->second.starttimeMaxCount=maxcount;
		}

		for(auto i=matchcountor.begin();i!=matchcountor.end();++i)
		{
			resinfo.AppendFormat(_T("found song:%d (%d time,startMatch %d)\n"),i->first,i->second.count,i->second.starttimeMaxCount);
		}
		MessageBox(resinfo);
		return S_OK;
	}
	LRESULT OnUploadData(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CUploadFreqData uploaddata;
		while(uploaddata.DoUpload()>0);
		return S_OK;
	}
	LRESULT OnSearchVarSite(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CSearchBySite serachbysite(freqinfos);
		if(serachbysite.StartSearch())
		{
			CAtlString resinfo;
			for(auto i=serachbysite.songresult.begin();i!=serachbysite.songresult.end();i++)
			{
				resinfo.AppendFormat(_T("song_id:%d match:%d name:%s\n"),i->song_id,i->match_count,i->filename);
			}
			MessageBox(resinfo);
		}
		return S_OK;
	}

	bool runing;
	LRESULT OnFileRecord(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		MMRESULT mmres;
		HWAVEIN hwi;
		WAVEHDR whdr,whdr1;
		UINT devId;
		WAVEFORMATEX wFormatEx;
		short outsample1[SampleCount];
		short outsample2[SampleCount];

		wFormatEx.wFormatTag = WAVE_FORMAT_PCM;
		wFormatEx.nChannels = 1;
		wFormatEx.wBitsPerSample = 16;
		wFormatEx.nSamplesPerSec = 44100;
		wFormatEx.nBlockAlign = wFormatEx.wBitsPerSample * wFormatEx.nChannels / 8;
		wFormatEx.nAvgBytesPerSec = wFormatEx.nSamplesPerSec* wFormatEx.nBlockAlign;

		// Open audio device
		debugstring(_T("\nwave in num %d\n"),waveInGetNumDevs());
		for (devId = 0; devId < waveInGetNumDevs(); devId++) {
			mmres = waveInOpen(&hwi, devId, &wFormatEx, (DWORD_PTR)CMainFrame::waveInProc,
				(DWORD_PTR)this, CALLBACK_FUNCTION);
			if (mmres == MMSYSERR_NOERROR) {
				break;
			}
		}
		if (mmres != MMSYSERR_NOERROR)
		{
			debugstring(_T("1-%d"),mmres);
			return S_FALSE;
		}

		ZeroMemory(&whdr, sizeof(WAVEHDR));
		whdr.lpData = (LPSTR)outsample1;
		whdr.dwBufferLength = sizeof(outsample1);

		ZeroMemory(&whdr1, sizeof(WAVEHDR));
		whdr1.lpData = (LPSTR)outsample2;
		whdr1.dwBufferLength = sizeof(outsample2);

		mmres = waveInPrepareHeader(hwi, &whdr, sizeof(WAVEHDR));
		if (mmres != MMSYSERR_NOERROR)
		{
			return FALSE;
		}

		mmres = waveInPrepareHeader(hwi, &whdr1, sizeof(WAVEHDR));
		if (mmres != MMSYSERR_NOERROR)
		{
			return FALSE;
		}
		
		mmres=waveInAddBuffer(hwi,&whdr,sizeof(WAVEHDR));
		mmres=waveInAddBuffer(hwi,&whdr1,sizeof(WAVEHDR));
		runing=true;
		dataline.clear();
		mmres=waveInStart(hwi);

		MessageBox(_T("点确定停止录音"));

		runing=false;
		mmres=waveInStop(hwi);
		mmres=waveInReset(hwi);
		mmres=waveInUnprepareHeader(hwi,&whdr, sizeof(WAVEHDR));
		mmres=waveInUnprepareHeader(hwi,&whdr1, sizeof(WAVEHDR));
		mmres=waveInClose(hwi);

		m_trackBar.SetRangeMax(100);
		m_trackBar.SetPos(50);
		this->BuildData();
		this->BuildImage();
		return S_OK;
	}
	static void CALLBACK waveInProc(
		HWAVEIN hwi,
		UINT uMsg,
		DWORD_PTR dwInstance,
		DWORD_PTR dwParam1,
		DWORD_PTR dwParam2
		)
	{
		CMainFrame *view=(CMainFrame*)dwInstance;
		if(view->runing && WIM_DATA==uMsg)
		{
			WAVEHDR *hdr=(WAVEHDR *)dwParam1;
			view->ProcessBuffer((short*)hdr->lpData,hdr->dwBytesRecorded/sizeof(short));
			MMRESULT mmres;
			hdr->dwFlags&=~WHDR_DONE;
			mmres = waveInAddBuffer(hwi, hdr, sizeof(WAVEHDR));
			if (mmres != MMSYSERR_NOERROR)
			{
				return ;
			}
		}
	}
	void ProcessBuffer(short *buffer,int count)
	{
		if(count!=SampleCount)
			return;
		std::vector<double> line;
		for(int i=0;i<count;i++)
			line.push_back(*(buffer+i));
		dataline.push_back(std::move(line));
	}
	
	std::vector<std::vector<double>> darklines;
	std::vector<FreqInfo> freqinfos;
	void BuildData()
	{
		darklines.clear();
		const int checkR=1;
		for(size_t i=checkR;i!=dataline.size()-checkR;i++)
		{
			std::vector<double> line(SampleCount/2);
			for(size_t j=checkR;j<SampleCount/2-checkR;j++)
			{
					/*const double core1[3][3]={
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
					line[j]=gpoint;*/
				const double core1[3][3]={
					{-1,-2,-1},
					{-2,13,-2},
					{-1,-2,-1}
				};
				double gx=0;
				for(int testi=-checkR;testi<checkR;testi++)
				{
					for(int testj=-checkR;testj<checkR;testj++)
					{
						double v=dataline[i+testi][j+testj];
						gx+=v*core1[testi+1][testj+1];
					}
				}
				if(gx>0)
					line[j]=gx;
			}
			darklines.push_back(std::move(line));
		}
		//darklines=dataline;
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
				if(strong>0.12)
				{
					double maxstrong=strong;
					for(int x=-area;x<=area;x++)
					{
						for(int y=-area;y<=area;y++)
						{
							maxstrong=max(maxstrong,darklines[i+x][j+y]);
							if(!(x==0 && y==0))
							{
								double other=darklines[i+x][j+y];
								if(other>strong)
								{
									goto NEXT;
								}
							}
						}
					}
					if(maxstrong==strong)
					{
						FreqInfo info;
						info.freq=j;
						info.time=i;
						info.strong=strong;
						freqinfos.push_back(info);
					}
					NEXT:;
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
				memimage.SetPixel(j,i,(BYTE)(strong*255+(1-strong)*back[0]),(BYTE)((1-strong)*back[1]),(BYTE)((1-strong)*back[2]));
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
		if(p.x<SampleCount/2 && p.y<(int)darklines.size())
		{
			CAtlString str;
			str.Format(_T("%d,%d==>%.5f"),p.x,p.y,darklines[p.y][p.x]);
			SetWindowText(str);
		}
		return S_OK;
	}
};
