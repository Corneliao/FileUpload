#include "QuickFramelessWindow.h"

#include <qforeach.h>
#include <windows.h>
#include <windowsx.h>
#include <QGuiApplication>

/* function --------------------------------------------------------------- 80 // ! ----------------------------- 120 */

WindowEffect::WindowEffect()
    : m_pSetWindowCompAttr(nullptr)
{
    // Dynamically load the (undocumented) API
    HMODULE hUser = GetModuleHandleW(L"user32.dll");

    if (hUser) {
        m_pSetWindowCompAttr = reinterpret_cast<PFN_SetWindowCompositionAttribute>(GetProcAddress(hUser, "SetWindowCompositionAttribute"));
    }

    if (!m_pSetWindowCompAttr) {
        qWarning() << "Failed to load SetWindowCompositionAttribute";
    }
}

void WindowEffect::setAcrylicEffect(HWND hWnd, const QColor& color, bool enableShadow, DWORD animId)
{
    if (!m_pSetWindowCompAttr)
        return;
    ACCENT_POLICY policy = {};
    policy.AccentState = ACCENT_ENABLE_ACRYLICBLURBEHIND;

    // Set shadow flags if requested (0x20|0x40|0x80|0x100) matches Python code
    policy.AccentFlags = enableShadow ? (0x20 | 0x40 | 0x80 | 0x100) : 0;

    // Use Qt’s ARGB directly (0xAARRGGBB)
    policy.GradientColor = static_cast<DWORD>(color.rgba());
    policy.AnimationId = animId;

    WINDOWCOMPOSITIONATTRIBDATA data = {};

    data.Attribute = WCA_ACCENT_POLICY;
    data.Data = &policy;
    data.SizeOfData = sizeof(policy);
    m_pSetWindowCompAttr(hWnd, &data);
}

void WindowEffect::setAeroEffect(HWND hWnd)
{
    if (!m_pSetWindowCompAttr)
        return;
    ACCENT_POLICY policy = {};
    policy.AccentState = ACCENT_ENABLE_BLURBEHIND;

    // other fields zero
    WINDOWCOMPOSITIONATTRIBDATA data = {};

    data.Attribute = WCA_ACCENT_POLICY;
    data.Data = &policy;
    data.SizeOfData = sizeof(policy);
    m_pSetWindowCompAttr(hWnd, &data);
}

void WindowEffect::disableEffect(HWND hWnd)
{
    if (!m_pSetWindowCompAttr)
        return;

    ACCENT_POLICY policy = {};
    policy.AccentState = ACCENT_DISABLED;

    WINDOWCOMPOSITIONATTRIBDATA data = {};
    data.Attribute = WCA_ACCENT_POLICY;
    data.Data = &policy;
    data.SizeOfData = sizeof(policy);
    m_pSetWindowCompAttr(hWnd, &data);
}

void WindowEffect::moveWindow(HWND hWnd)
{
    // Release mouse capture & send WM_SYSCOMMAND to drag the title bar
    ReleaseCapture();
    SendMessageW(hWnd, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, 0);
}

QuickFramelessWindow::QuickFramelessWindow(QObject* parent)
{
    this->initStyle();
    this->setTextRenderType(QQuickWindow::CurveTextRendering);
}

QuickFramelessWindow::~QuickFramelessWindow()
{
    if (this->whitelistItems.count() != 0) {
        this->whitelistItems.clear();
    }
}

void QuickFramelessWindow::setWindowTitleBar(QQuickItem* item)
{
    if (item)
        this->titleBar = item;
}

void QuickFramelessWindow::moveCenter(bool isVisible)
{
    this->setX((this->screenSize.width() - this->width()) / 2);
    this->setY((this->screenSize.height() - this->height()) / 2);
    this->setVisible(isVisible);
}

//
void QuickFramelessWindow::showWindowState(const QString& state)
{
    //
    HWND hwnd = reinterpret_cast<HWND>(this->winId());
    if (state == "Full") {
        if (this->windowState() == Qt::WindowState::WindowFullScreen)
            this->showNormal();
        else
            this->showFullScreen();
    }
    if (state == "Max") {
        if (this->windowState() == Qt::WindowState::WindowMaximized)
            ShowWindow(hwnd, SW_SHOWNORMAL);
        else
            ShowWindow(hwnd, SW_SHOWMAXIMIZED);
    }
    if (state == "Min") {
        ShowWindow(hwnd, SW_SHOWMINIMIZED);
    }
    if (state == "Show") {
        this->setVisible(true);
        ShowWindow(hwnd, SW_SHOWDEFAULT);
    }
    if (state == "Close") {
        CloseWindow(hwnd);
    }
}

Qt::WindowState QuickFramelessWindow::currentWindowState()
{
    return this->windowState();
}

void QuickFramelessWindow::setMainLayout(QQuickItem* item)
{
    if (item) {
        this->mainLayoutItem = item;
    }
}

void QuickFramelessWindow::setAvctiveWindow()
{
    HWND hwnd = reinterpret_cast<HWND>(this->winId());
    SetActiveWindow(hwnd);
}

void QuickFramelessWindow::addWhiteItem(QQuickItem* item)
{
    if (item) {
        this->whitelistItems.append(item);
    }
}

void QuickFramelessWindow::addWhiteItems(QList<QQuickItem*> items)
{
    foreach (QQuickItem* item, items) {
        this->whitelistItems.append(item);
    }
}

void QuickFramelessWindow::disableDoubleClickedMaxWindow(bool is_disable)
{
    this->m_disable = is_disable;
}

void QuickFramelessWindow::setBackgroundColor(const QColor& color)
{
    setColor(color);
}

/**
 * @brief QuickFramelessWindow::initStyle
 */
void QuickFramelessWindow::initStyle()
{
    this->dpr = this->devicePixelRatio();

    this->screenSize = QGuiApplication::primaryScreen()->size();

    // set Window flags
    this->setFlags(this->flags() | Qt::WindowType::Window | Qt::FramelessWindowHint);

    HWND hwnd = reinterpret_cast<HWND>(this->winId());
    LONG style = GetWindowLong(hwnd, GWL_STYLE);
    SetWindowLong(hwnd, GWL_STYLE, style | WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX);

    UINT preference = DWMWCP_ROUND;

    DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &preference, sizeof(preference));
}

bool QuickFramelessWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)
{
    MSG* msg = static_cast<MSG*>(message);
    switch (msg->message) {
    case WM_NCCALCSIZE: {
        NCCALCSIZE_PARAMS& params = *reinterpret_cast<NCCALCSIZE_PARAMS*>(msg->lParam);
        if (params.rgrc[0].top != 0)
            params.rgrc[0].top -= -1;
        // 确保窗口内容区域正确计算，减少缩放时的视觉闪烁
        *result = WVR_REDRAW;
        return true;
    }
    case WM_NCHITTEST: {
        int x = GET_X_LPARAM(msg->lParam);
        int y = GET_Y_LPARAM(msg->lParam);
        QPoint point(x, y);
        *result = this->adjustResizeWindow(msg->hwnd, point);
        if (*result != 0)
            return true;
        *result = this->TackingWindowTitleBar(point);
        if (*result != 0)
            return true;
        return false;
    }
    case WM_NCLBUTTONDBLCLK: {
        if (msg->wParam == HTCAPTION) {
            if (this->m_disable) {
                *result = 0; // 阻止默认最大化行为
                return true; // 事件已处理
            }
        }
        return QQuickWindow::nativeEvent(eventType, message, result);
    }

    default:
        return QQuickWindow::nativeEvent(eventType, message, result);
    }
}

bool QuickFramelessWindow::event(QEvent* event)
{
    if (event->type() == QEvent::WindowStateChange) {
        RECT rect = {0, 0, 0, 0};
        AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, 0);

        rect.left = std::abs(rect.left);
        rect.top = std::abs(rect.left);
        rect.right = std::abs(rect.left);
        rect.bottom = std::abs(rect.left);

        if (this->mainLayoutItem == nullptr)
            return QQuickWindow::event(event);

        int instence = std::abs(static_cast<int>(rect.left / this->dpr));

        if (this->windowState() == Qt::WindowState::WindowMaximized) {
            QQmlProperty(this->mainLayoutItem, "anchors.margins").write(instence);
        } else if (this->windowState() == Qt::WindowState::WindowFullScreen) {
            QQmlProperty(this->mainLayoutItem, "anchors.margins").write(0);
        } else {
            QQmlProperty(this->mainLayoutItem, "anchors.margins").write(0);
        }
    }

    if (event->type() == QEvent::Close) {
        emit this->windowClose();
    }

    return QQuickWindow::event(event);
}

qintptr QuickFramelessWindow::adjustResizeWindow(const HWND& hwnd, const QPoint& point)
{
    int result = 0;
    RECT rect = {0, 0, 0, 0};
    GetWindowRect(hwnd, &rect);

    int x = point.x();
    int y = point.y();

    bool resize_width = this->minimumWidth() != this->maximumWidth();
    bool resize_height = this->minimumHeight() != this->maximumHeight();

    if (resize_width) {
        if (x >= rect.left && x < rect.left + RESIZEWINDOWBORDERWIDTH)
            result = HTLEFT;
        if (x <= rect.right && x > rect.right - RESIZEWINDOWBORDERWIDTH)
            result = HTRIGHT;
    }
    if (resize_height) {
        if (y >= rect.top && y < rect.top + RESIZEWINDOWBORDERWIDTH)
            result = HTTOP;
        if (y <= rect.bottom && y > rect.bottom - RESIZEWINDOWBORDERWIDTH)
            result = HTBOTTOM;
    }

    if (resize_width && resize_height) {
        if (x >= rect.left && x < rect.left + RESIZEWINDOWBORDERWIDTH && y >= rect.top && y < rect.top + RESIZEWINDOWBORDERWIDTH)
            result = HTTOPLEFT;
        if (x <= rect.right && x > rect.right - RESIZEWINDOWBORDERWIDTH && y >= rect.top && y < rect.top + RESIZEWINDOWBORDERWIDTH)
            result = HTTOPRIGHT;
        if (x >= rect.left && x < rect.left + RESIZEWINDOWBORDERWIDTH && y <= rect.bottom && y > rect.bottom - RESIZEWINDOWBORDERWIDTH)
            result = HTBOTTOMLEFT;
        if (x <= rect.right && x > rect.right - RESIZEWINDOWBORDERWIDTH && y <= rect.bottom && y > rect.bottom - RESIZEWINDOWBORDERWIDTH)
            result = HTBOTTOMRIGHT;
    }

    return result;
}

qintptr QuickFramelessWindow::TackingWindowTitleBar(const QPoint& point)
{
    int result = 0;
    if (titleBar == nullptr)
        return result;
    QPointF pointF = titleBar->mapFromGlobal(QPointF(point.x() / dpr, point.y() / dpr));
    if (!titleBar->contains(pointF))
        return result;
    QQuickItem* child = titleBar->childAt(pointF.x(), pointF.y());
    if (!child)
        result = HTCAPTION;
    else {
        if (whitelistItems.contains(child))
            result = HTCAPTION;
    }

    return result;
}

void QuickFramelessWindow::enableShadow()
{
    this->setColor(Qt::transparent);
    HWND hwnd = reinterpret_cast<HWND>(winId());
    effect.setAcrylicEffect(hwnd);
}

void QuickFramelessWindow::disableAcrylicEffect()
{
    this->setColor(Qt::white);
    HWND hwnd = reinterpret_cast<HWND>(winId());
    effect.disableEffect(hwnd);
}
