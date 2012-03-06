// FreqWatchView.h : interface of the CFreqWatchView class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include "DIBBitmap.h"
static const size_t SampleCount=4096;
const int WM_CLIENTMOUSEMOVE=WM_USER+1;
struct FreqInfo
{
	int freq;
	int time;
	double strong;
};
class CFreqWatchView : public CScrollWindowImpl<CFreqWatchView>
{
public:
	DECLARE_WND_CLASS(NULL)

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		pMsg;
		return FALSE;
	}

	BEGIN_MSG_MAP_EX(CFreqWatchView)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		CHAIN_MSG_MAP(CScrollWindowImpl<CFreqWatchView>)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
	CDIBBitmap *image;
	std::vector<FreqInfo> *freqinfos;
	void DoPaint(CDCHandle dc)
	{
		/*if(image->IsNull()==FALSE)
		{
			image->BitBlt(dc,0,0);
		}*/
		image->SetDIBitsToDevice(dc,0,0);

		CPen pen;
		CPen srcpen=pen.CreatePen(PS_SOLID,2,RGB(0,0,0));
		dc.SelectPen(pen);

		auto end=freqinfos->end();
		for(auto i=freqinfos->begin();i!=end;i++)
		{
			dc.MoveTo(i->freq-3,i->time-3);
			dc.LineTo(i->freq+3,i->time+3);
			dc.MoveTo(i->freq-3,i->time+3);
			dc.LineTo(i->freq+3,i->time-3);
		}

		CPen linepen;
		linepen.CreatePen(PS_SOLID,1,RGB(0,0,255));
		dc.SelectPen(linepen);
		CSize scsize;
		GetScrollSize(scsize);
		for(size_t j=20;j<SampleCount/2;j+=40)
		{
			dc.MoveTo(j,0);
			dc.LineTo(j,scsize.cy-1);
		}
		
		dc.SelectPen(srcpen);
	}
	void OnMouseMove(UINT nFlags, CPoint point)
	{
		
		::SendMessage(this->GetParent(),WM_CLIENTMOUSEMOVE,(WPARAM)point.x+m_ptOffset.x,(LPARAM)point.y+m_ptOffset.y);
	}
};
