//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of PXI plot for plotting 2 values
//

#include <math.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP

#include "pxiplot2.h"

#include "gmisc.h"
#include "wxmisc.h"
#include "expdata.h"
#include "equtrac.h"
#include "pxi.h"

#define NUM_COLORS	11
static char *equ_colors[NUM_COLORS+1] =
{"BLACK","RED","BLUE","GREEN","CYAN","VIOLET","MAGENTA","ORANGE",
 "PURPLE","PALE GREEN","BROWN","BLACK"}; // not pretty

//***************************** PLOT LABELS ********************************
void PxiPlot2::PlotLabels(wxDC &dc, int ch,int cw)
{
  dc.SetTextForeground(*wxBLACK);
  //dc.SetBackgroundMode(wxTRANSPARENT);
  dc.SetFont(m_drawSettings.GetLabelFont());
  for (int i=1;i<=labels.Length();i++) {
    wxCoord tw,th;
    dc.GetTextExtent(labels[i].label,&tw,&th);
    dc.DrawText(labels[i].label,(int) (labels[i].x*cw-tw/2),
		(int) (labels[i].y*ch-th/2));
  }
}

void PxiPlot2::DrawExpPoint_2(wxDC &dc, const PlotInfo &thisplot, double cur_e, 
			       int pl1,int st1,int pl2,int st2,
			       int x0, int y0, int cw, int ch)
{
  exp_data_struct	*s=0;
  double x,y;
  gBlock<int> point_nums;
  
  point_nums=exp_data->HaveL(cur_e);
  for (int i=1;i<=point_nums.Length();i++) {
    s=(*exp_data)[point_nums[i]];
    
    x=x0+(*s).probs[pl1][st1]*cw;
    y=y0-(*s).probs[pl2][st2]*ch;
    
    if (m_drawSettings.GetOverlaySym()==OVERLAY_TOKEN) {
      int ts=m_drawSettings.GetTokenSize();	// token dimentions are 2ts x 2ts
      dc.SetBrush(m_drawSettings.GetDataBrush());
      dc.DrawEllipse((int) (x-ts), (int) (y-ts), 2*ts, 2*ts);
    }
    else {
      char tmp[10];
      sprintf(tmp,"%d",point_nums[i]);
      dc.SetFont(m_drawSettings.GetOverlayFont());
      dc.SetTextForeground(*wxBLACK);
      wxCoord tw,th;
      dc.GetTextExtent(tmp,&tw,&th);
      dc.DrawText(tmp, (int) (x-tw/2), (int) (y-th/2));
      //      dc.DrawText(tmp,x-3,y-6);
    }
    delete s;
  }
}

void PxiPlot2::PlotData_2(wxDC& dc,const PlotInfo &thisplot,int x0, int y0, int cw,int ch,
			     const FileHeader &f_header,int level)
{
  double x,y;
  int iset;
  int point_color=1; // color of the pixel, corresponds to equilibrium #
  static int color_start;		// makes each overlay file be plotted a different color set
  int max_equ=0;
  int new_equ=0;
  if (level==1) color_start=0;
  
  // where the actual data gets read in
  gFileInput f(f_header.FileName());
  DataLine *prev_point=(m_drawSettings.ConnectDots()) ? new DataLine : NULL;
  if (!FindStringInFile(f, "Data:")) {
    return;
  }

  EquTracker equs;		// init the EquTracker class
  DataLine probs(f_header.FileName());
  f>>probs;
  // Figure out what strategies I am plotting.
  int pl1=0,st1=0,pl2=0,st2=0;
  for (int j=1;j<=f_header.NumInfosets();j++)
    for (int i=1;i<=f_header.NumStrategies(j);i++)
      if (thisplot.GetStrategyShow(j,i))
	if (pl1==0) {pl1=j;st1=i;} else {pl2=j;st2=i;}
  
  while (!probs.Done() && !f.eof()) {
    if (probs.E() < m_probAxisProp.m_scale.GetMaximum() && 
	probs.E() > m_probAxisProp.m_scale.GetMinimum()) {
      point_color=equs.Check_Equ(probs,&new_equ,prev_point);
      dc.SetPen(*(wxThePenList->FindOrCreatePen(equ_colors[point_color%NUM_COLORS+1],3,wxSOLID)));

      x=x0+probs[pl1][st1]*cw;
      y=y0-probs[pl2][st2]*ch;
      if (m_drawSettings.ConnectDots() && !new_equ) {
	double prev_x=x0+(*prev_point)[pl1][st1]*cw;
	double prev_y=y0-(*prev_point)[pl2][st2]*ch;
	dc.DrawLine((int) prev_x, (int) prev_y, (int) x, (int) y);
      }
      else {
	dc.DrawPoint((int) x, (int) y);
      }
      dc.DrawPoint((int) x, (int) y);
      // if there is an experimental data point for this cur_e, plot it
      if (exp_data) {
	DrawExpPoint_2(dc,thisplot,probs.E(),pl1,st1,pl2,st2, x0,y0,cw,ch);
      }
    }
    f>>probs;
  }
  if (prev_point) delete prev_point;
  PlotLabels(dc,ch,cw);
}

void PxiPlot2::PlotAxis_2(wxDC& dc, const PlotInfo &thisplot, 
			  int x0, int y0, int cw,int ch)
{
#ifdef NOT_PORTED_YET
  int i1;
  float tempf;
  char axis_label_str[90];
  int num_plots=0;
  float x_start = thisplot.GetMinY();
  float x_end = thisplot.GetMaxY();
  float y_start = thisplot.GetMinY();
  float y_end = thisplot.GetMaxY();

  // temporary replacement of old constant; should be made configurable
  const int XGRIDS = 10;
  int XGRID = XGRIDS / cw;

  if (thisplot.ShowAxis()) {
    dc.DrawLine(x0, y0, x0, y0-ch);
    dc.DrawLine(x0, y0, x0+cw, y0);}
  if (thisplot.ShowSquare()) {
    dc.DrawLine(x0+cw, y0-ch, x0, y0-ch);
    dc.DrawLine(x0+cw, y0-ch, x0+cw, y0);}
  for (i1=0;i1<=XGRIDS;i1++) {
    if (thisplot.ShowTicks()) {
      dc.DrawLine(x0+i1*XGRID, y0-GRID_H, x0+i1*XGRID, y0+GRID_H);
      if (thisplot.ShowSquare())
	dc.DrawLine( x0+i1*XGRID, y0-ch-GRID_H, x0+i1*XGRID,  y0-ch+GRID_H);
    }
    if (thisplot.ShowNums()) {
      tempf=x_start+fabs((x_start-x_end)/XGRIDS)*i1;
      sprintf(axis_label_str,"%2.1f",tempf);
      wxCoord tw,th;
      dc.GetTextExtent(axis_label_str,&tw,&th);
      dc.DrawText(axis_label_str,
		  x0+i1*XGRID-tw/2, y0+GRID_H);
      if (thisplot.ShowSquare())
	dc.DrawText(axis_label_str,
		    x0+i1*XGRID-tw/2,y0-ch-GRID_H-th);
    }
  }
  for (i1=0;i1<=YGRIDS;i1++) {
    if (thisplot.ShowTicks()) {
      dc.DrawLine(
		  x0-GRID_H,   y0-ch+i1*YGRID,
		  x0+GRID_H,   y0-ch+i1*YGRID);
      if (thisplot.ShowSquare())
	dc.DrawLine(
		    x0+cw-GRID_H,  y0-ch+i1*YGRID,
		    x0+cw+GRID_H,  y0-ch+i1*YGRID);
    }
    if (thisplot.ShowNums()) {
      tempf=y_end-fabs((y_start-y_end)/YGRIDS)*i1;
      sprintf(axis_label_str,"%3.2f",tempf);
      wxCoord tw,th;
      dc.GetTextExtent(axis_label_str,&tw,&th);
      dc.DrawText(axis_label_str,
		  x0-GRID_H-tw,   y0-ch+i1*YGRID-th/2);
      if (thisplot.ShowSquare())
	dc.DrawText(axis_label_str,
		    x0+cw+GRID_H, y0-ch+i1*YGRID-th/2);
    }
  }

#endif // NOT_PORTED_YET
}

void PxiPlot2::DoPlot(wxDC& dc, const PlotInfo &thisplot,
			 int x0, int y0, int cw,int ch, int level)
{
  PlotAxis_2(dc,thisplot,x0,y0,cw,ch);
  PlotData_2(dc,thisplot,x0,y0,cw,ch, m_header, 1);
}

void PxiPlot2::OnEvent(wxMouseEvent &ev)
{
  if (ev.ShiftDown() && ev.ButtonDown()) {  // use shift mouse click to add a label
    int w,h;
    GetClientSize(&w,&h);
    label_struct tmp_label;
    //    tmp_label.x=ev.x/w;tmp_label.y=ev.y/h;
    tmp_label.x=ev.GetX()/w;tmp_label.y=ev.GetY()/h;
    // Check if clicked on an already existing text
    int clicked_on=0;
    for (int i=1;i<=labels.Length();i++)
      if (labels[i].x+TEXT_MARGIN>tmp_label.x && labels[i].x-TEXT_MARGIN<tmp_label.x &&
	  labels[i].y+TEXT_MARGIN>tmp_label.y && labels[i].y-TEXT_MARGIN<tmp_label.y)
	clicked_on=i;
    const char * junk = (clicked_on) ? (const char *)labels[clicked_on].label : "";
    tmp_label.label=wxGetTextFromUser("Enter Label","Enter Label", (char *)junk);
    //					  (clicked_on) ? (char *)labels[clicked_on].label : "");
    if (!clicked_on)
      labels.Append(tmp_label);
    else {
      tmp_label.x=labels[clicked_on].x;
      tmp_label.y=labels[clicked_on].y;
      labels[clicked_on]=tmp_label;
    }
    Render();
  }
}

BEGIN_EVENT_TABLE(PxiPlot2, wxScrolledWindow)
  EVT_MOUSE_EVENTS(PxiPlot2::OnEvent)
END_EVENT_TABLE()

PxiPlot2::PxiPlot2(wxWindow *p_parent, const wxPoint &p_position,
		     const wxSize &p_size,
		     const FileHeader &p_header, int p_page)
  : PxiPlot(p_parent, p_position, p_size, p_header, p_page)
{ }

PxiPlot2::~PxiPlot2()
{ }

