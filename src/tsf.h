/* imported from the skkime */
#if !defined (tsf_h)
#define	tsf_h

#define	ARRAYSIZE(array)	(sizeof (array) / sizeof (array[0]))

inline void SafeStringCopy (WCHAR *pchDst, ULONG cchMax, const WCHAR *pchSrc)
{
    if (cchMax > 0)
    {
        wcsncpy(pchDst, pchSrc, cchMax);
        pchDst[cchMax-1] = '\0';
    }
}

typedef HRESULT (WINAPI *PTF_CREATETHREADMGR)(ITfThreadMgr**) ;
typedef HRESULT (WINAPI *PTF_CREATELANGBARITEMMGR)(ITfLangBarItemMgr**);
typedef struct {
	const WCHAR*	pchDesc ;
	void	(*pfnHandler)(void) ;
}	TSFLBMENUINFO ;

extern const CLSID	c_clsidSaenaruTextService ;
extern const GUID	c_guidSaenaruProfile ;
extern const GUID	c_guidItemButtonCMode ;
extern const GUID	c_guidItemButtonIME ;
extern const GUID	c_guidItemButtonShape ;
extern const GUID	c_guidKeyboardItemButton ;

/*	prototypes */
HIMC	_GetCurrentHIMC		(void) ;
BOOL	CreateItemButtonCMode	(ITfLangBarItem** ppLangBarItem) ;
BOOL	CreateItemButtonIME	(ITfLangBarItem** ppLangBarItem) ;
BOOL	CreateItemButtonShape	(ITfLangBarItem** ppLangBarItem) ;
BOOL	_ShowKeyboardIcon	(BOOL fShow) ;

LONG DllAddRef(void);
LONG DllRelease(void);

#endif

