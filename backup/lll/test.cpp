void COpenGLControl::OnSize(UINT nType, int cx, int cy)
{
wglMakeCurrent(hdc, hrc);
CWnd::OnSize(nType, cx, cy);

if (0 >= cx || 0 >= cy || nType == SIZE_MINIMIZED) return;
oglWindowWidth = cx;
oglWindowHeight = cy;
// Map the OpenGL coordinates.
glViewport(0, 0, oglWindowWidth, oglWindowHeight);

// Projection view
glMatrixMode(GL_PROJECTION);
glLoadIdentity();
// Set our current view perspective
glOrtho(-oglWindowWidth/2, oglWindowWidth/2, oglWindowHeight/2,-oglWindowHeight/2, -1, 1);
// Model view
glMatrixMode(GL_MODELVIEW);
wglMakeCurrent(NULL, NULL);
}  

void COpenGLControl::OnDraw(CDC *pDC)
{
wglMakeCurrent(hdc,hrc);
// If the current view is perspective...
glLoadIdentity();
glScalef(m_fZoom,m_fZoom,1.0);
glTranslatef(m_fPosX, m_fPosY, 0.0f);
wglMakeCurrent(NULL, NULL);
}  

void COpenGLControl::OnMouseMove(UINT nFlags, CPoint point)
{
// TODO: Add your message handler code here and/or call default
int diffX = (int)(point.x - m_fLastX);
int diffY = (int)(point.y - m_fLastY);
m_fLastX = (float)point.x;
m_fLastY = (float)point.y;
if (nFlags & MK_LBUTTON)
{
    m_fPosX += (float)0.5f * diffX;
    m_fPosY -= (float)0.5f * diffY;
}
OnDraw(NULL);
CWnd::OnMouseMove(nFlags, point);
}  