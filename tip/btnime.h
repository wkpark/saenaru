
class CLangBarItemImeButton : public ITfLangBarItemButton, public ITfSource
{
  public:
    CLangBarItemImeButton(CSaenaruTextService *pSaenaru);
    ~CLangBarItemImeButton();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    // ITfLangBarItem
    STDMETHODIMP GetInfo(TF_LANGBARITEMINFO *pInfo);
    STDMETHODIMP GetStatus(DWORD *pdwStatus);
    STDMETHODIMP Show(BOOL fShow);
    STDMETHODIMP GetTooltipString(BSTR *pbstrToolTip);

    // ITfLangBarItemButton
    STDMETHODIMP OnClick(TfLBIClick click, POINT pt, const RECT *prcArea);
    STDMETHODIMP InitMenu(ITfMenu *pMenu);
    STDMETHODIMP OnMenuSelect(UINT wID);
    STDMETHODIMP GetIcon(HICON *phIcon);
    STDMETHODIMP GetText(BSTR *pbstrText);

    // ITfSource
    STDMETHODIMP AdviseSink(REFIID riid, IUnknown *punk, DWORD *pdwCookie);
    STDMETHODIMP UnadviseSink(DWORD dwCookie);

    // Misc
    void _OnUpdate(DWORD);

    // Misc menu methods
    static void _Menu_Help(CLangBarItemImeButton *, UINT);
    static void _Menu_Property(CLangBarItemImeButton *, UINT);
    static void _Menu_Reconversion(CLangBarItemImeButton *, UINT);
    static void _Menu_ToggleShowKeyboard(CLangBarItemImeButton *, UINT);
    static void _Menu_ToggleDvorak(CLangBarItemImeButton *, UINT);
    static void _Menu_ToggleEscEng(CLangBarItemImeButton *, UINT);
    static void _Menu_ToggleHanEng(CLangBarItemImeButton *, UINT);
    static void _Menu_ToggleOnTheSpot(CLangBarItemImeButton *, UINT);
    static void _Menu_SelectKeyboard(CLangBarItemImeButton *, UINT);
    static void _Menu_SelectCompose(CLangBarItemImeButton *, UINT);
    static DWORD _MenuItem_GetNormalFlag(CLangBarItemImeButton *, UINT);
    static DWORD _MenuItem_GetKeyboardFlag(CLangBarItemImeButton *, UINT);
    static DWORD _MenuItem_GetComposeFlag(CLangBarItemImeButton *, UINT);
    static DWORD _MenuItem_GetToggleKeyboardFlag(CLangBarItemImeButton *, UINT);
    static DWORD _MenuItem_GetToggleComposeFlag(CLangBarItemImeButton *, UINT);
    static DWORD _MenuItem_GetToggleDvorakFlag(CLangBarItemImeButton *, UINT);
    static DWORD _MenuItem_GetToggleEscEngFlag(CLangBarItemImeButton *, UINT);
    static DWORD _MenuItem_GetToggleHanEngFlag(CLangBarItemImeButton *, UINT);
    static DWORD _MenuItem_GetToggleOnTheSpotFlag(CLangBarItemImeButton *, UINT);
    static DWORD _UserKeyboardMenu_GetKeyboardFlag(CLangBarItemImeButton *, UINT);

  private:
    CSaenaruTextService *_pSaenaru;
    ITfLangBarItemSink *_pLangBarItemSink;
    TF_LANGBARITEMINFO _tfLangBarItemInfo;
    LONG _cRef;
};

/*
 * vim: et sts=4 sw=4
 */
