//
// saenarutip.h
//
// CSaenaruTextService declaration.
//

#ifndef SAENARUTIP_H
#define SAENARUTIP_H

#define CMODE_ALPHANUMERIC  0x0000
#define CMODE_HANGUL        0x0001
#define CMODE_HANJA         0x0002
#define CMODE_HIRAGANA      0x0004
#define CMODE_FULLSHAPE     0x0008
#define CMODE_ROMAN         0x0010
#define CMODE_CHARCODE      0x0020
#define CMODE_TOOLBAR       0x0100

class CLangBarItemButton;
class CLangBarItemShapeButton;
class CLangBarItemImeButton;

class CSaenaruTextService : public ITfTextInputProcessor,
                         public ITfDisplayAttributeProvider,
                         public ITfCreatePropertyStore,
                         public ITfThreadMgrEventSink,
                         public ITfTextEditSink,
                         public ITfCompositionSink,
                         public ITfCleanupContextDurationSink,
                         public ITfCleanupContextSink,
                         public ITfCompartmentEventSink,
                         public ITfKeyEventSink
{
public:
    CSaenaruTextService();
    ~CSaenaruTextService();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    // ITfTextInputProcessor
    STDMETHODIMP Activate(ITfThreadMgr *pThreadMgr, TfClientId tfClientId);
    STDMETHODIMP Deactivate();

    // ITfDisplayAttributeProvider
    STDMETHODIMP EnumDisplayAttributeInfo(IEnumTfDisplayAttributeInfo **ppEnum);
    STDMETHODIMP GetDisplayAttributeInfo(REFGUID guidInfo, ITfDisplayAttributeInfo **ppInfo);

    // ITfCreatePropertyStore
    STDMETHODIMP IsStoreSerializable(REFGUID guidProperty, ITfRange *pRange, ITfPropertyStore *pPropertyStore, BOOL *pfSerializable);
    STDMETHODIMP CreatePropertyStore(REFGUID guidProperty, ITfRange *pRange, ULONG cb, IStream *pStream, ITfPropertyStore **ppStore);

    // ITfThreadMgrEventSink
    STDMETHODIMP OnInitDocumentMgr(ITfDocumentMgr *pDocMgr);
    STDMETHODIMP OnUninitDocumentMgr(ITfDocumentMgr *pDocMgr);
    STDMETHODIMP OnSetFocus(ITfDocumentMgr *pDocMgrFocus, ITfDocumentMgr *pDocMgrPrevFocus);
    STDMETHODIMP OnPushContext(ITfContext *pContext);
    STDMETHODIMP OnPopContext(ITfContext *pContext);

    // ITfTextEditSink
    STDMETHODIMP OnEndEdit(ITfContext *pContext, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord);

    // ITfCompositionSink
    STDMETHODIMP OnCompositionTerminated(TfEditCookie ecWrite, ITfComposition *pComposition);

    // ITfCleanupContextDurationSink
    STDMETHODIMP OnStartCleanupContext();
    STDMETHODIMP OnEndCleanupContext();

    // ITfCleanupContextSink
    STDMETHODIMP OnCleanupContext(TfEditCookie ecWrite, ITfContext *pContext);

    // ITfCompartmentEventSink
    STDMETHODIMP OnChange(REFGUID rguidCompartment);

    // ITfKeyEventSink
    STDMETHODIMP OnSetFocus(BOOL fForeground);
    STDMETHODIMP OnTestKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten);
    STDMETHODIMP OnKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten);
    STDMETHODIMP OnTestKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten);
    STDMETHODIMP OnKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten);
    STDMETHODIMP OnPreservedKey(ITfContext *pContext, REFGUID rguid, BOOL *pfEaten);

    // CClassFactory factory callback
    static HRESULT CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj);

    // server registration
    static BOOL RegisterProfiles();
    static void UnregisterProfiles();
    static BOOL RegisterCategories(BOOL fRegister);
    static BOOL RegisterServer();
    static void UnregisterServer();

    // Conversion Mode
    DWORD GetConversionStatus();
    DWORD SetConversionStatus(DWORD status);

    // language bar menu handlers
    static void _Menu_OnComposition(CSaenaruTextService *_this);
    static void _Menu_OnSetGlobalCompartment(CSaenaruTextService *_this);
    static void _Menu_OnSetCaseProperty(CSaenaruTextService *_this);
    static void _Menu_OnViewCaseProperty(CSaenaruTextService *_this);
    static void _Menu_OnViewCustomProperty(CSaenaruTextService *_this);
    static void _Menu_OnSetCustomProperty(CSaenaruTextService *_this);

    // language bar menu, toggle mode handlers
    static void _Menu_ToHangul(CSaenaruTextService *_this);
    static void _Menu_ToAscII(CSaenaruTextService *_this);
    static void _Menu_ToHanja(CSaenaruTextService *_this);

    // keystroke handlers
    HRESULT _HandleKeyDown(TfEditCookie ec, ITfContext *pContext, WPARAM wParam);
    HRESULT _HandleArrowKey(TfEditCookie ec, ITfContext *pContext, WPARAM wParam);
    HRESULT _HandleReturn(TfEditCookie ec, ITfContext *pContext);
    HRESULT _HandleBackspace(TfEditCookie ec, ITfContext *pContext);
    // hangul utils
    HRESULT _HangulCommitComposition(TfEditCookie ec, ITfContext *pContext, WPARAM wParam = 0);
    WCHAR _HangulCommit();
    WCHAR _HangulPreedit();

    // some utils
    WORD _ConvertVKey(UINT code);
    void _UpdateLanguageBar(DWORD dwFlags);

    // callbacks for CCompositionEditSession
    BOOL _IsComposing()
    {
        return _pComposition != NULL;
    }
    ITfComposition *_GetComposition()
    {
        return _pComposition;
    }
    void _SetComposition(ITfComposition *pComposition)
    {
        _pComposition = pComposition;
    }
    void _TerminateComposition(TfEditCookie ec)
    {
        if (_pComposition != NULL)
        {
            _ClearCompositionDisplayAttributes(ec);
            _pComposition->EndComposition(ec);
            SafeReleaseClear(_pComposition);
        }
    }
    void _StartCompositionInContext(ITfContext *pContext, WPARAM wParam = 0);
    void _EndCompositionInContext(ITfContext *pContext, WPARAM wParam = 0);

    void _ClearCompositionDisplayAttributes(TfEditCookie ec);
    BOOL _SetCompositionDisplayAttributes(TfEditCookie ec, TfGuidAtom gaDisplayAttribute = 0);

    // messages for the worker wnd
    enum { WM_DISPLAY_PROPERTY = WM_USER + 1 };
    // property methods
    void _ViewCaseProperty(TfEditCookie ec, ITfContext *pContext);
    void _SetCaseProperty(TfEditCookie ec, ITfContext *pContext);
    void _ViewCustomProperty(TfEditCookie ec, ITfContext *pContext);
    void _SetCustomProperty(TfEditCookie ec, ITfContext *pContext);

private:
    // init methods
    BOOL _InitLanguageBar();
    BOOL _InitThreadMgrSink();
    BOOL _InitTextEditSink(ITfDocumentMgr *pDocMgr);
    BOOL _InitDisplayAttributeGuidAtom();
    BOOL _InitCleanupContextDurationSink();
    BOOL _InitCleanupContextSink(ITfContext *pContext);
    BOOL _InitContextCompartment(ITfContext *pContext);
    BOOL _InitGlobalCompartment();
    BOOL _InitWorkerWnd();
    BOOL _InitKeystrokeSink();
    BOOL _InitPreservedKey();

    // uninit methods
    void _UninitLanguageBar();
    void _UninitThreadMgrSink();
    void _UninitCleanupContextDurationSink();
    void _UninitCleanupContextSink(ITfContext *pContext);
    void _UninitCompartment(ITfContext *pContext);
    void _UninitGlobalCompartment();
    void _UninitWorkerWnd();
    void _UninitKeystrokeSink();
    void _UninitPreservedKey();

    // property methods
    void _RequestPropertyEditSession(ULONG ulCallback);
    static LRESULT CALLBACK _WorkerWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    //
    // state
    //

    ITfThreadMgr *_pThreadMgr;
    TfClientId _tfClientId;

    ITfComposition *_pComposition; // pointer to an ongoing composition, or NULL if there is none

    BOOL _fCleaningUp;

    TfGuidAtom _gaDisplayAttributeInput;
    TfGuidAtom _gaDisplayAttributeConverted;

    CLangBarItemButton *_pLangBarItem;
    CLangBarItemButton *_pLangBarModeItem;
    CLangBarItemShapeButton *_pLangBarShapeItem;
    CLangBarItemImeButton *_pLangBarImeItem;

    DWORD _dwThreadMgrEventSinkCookie;
    DWORD _dwThreadFocusSinkCookie;
    DWORD _dwTextEditSinkCookie;
    DWORD _dwGlobalCompartmentEventSinkCookie;

    DWORD _dwConversionStatus;

    ITfContext *_pTextEditSinkContext;

    HWND _hWorkerWnd;
    // buffers for the popup property displays
    WCHAR _achDisplayText[64];
    WCHAR _achDisplayPropertyText[64];

    LONG _cRef;     // COM ref count
};


#endif // SAENARUTIP_H

/*
 * vim: et sts=4 sw=4
 */
