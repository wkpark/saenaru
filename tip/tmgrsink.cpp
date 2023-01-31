//
// tmgrsink.cpp
//
// ITfThreadMgrEventSink implementation.
//

#include "globals.h"
#include "saenarutip.h"

//+---------------------------------------------------------------------------
//
// OnInitDocumentMgr
//
// Sink called by the framework just before the first context is pushed onto
// a document.
//----------------------------------------------------------------------------

STDAPI CSaenaruTextService::OnInitDocumentMgr(ITfDocumentMgr *pDocMgr)
{
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// OnUninitDocumentMgr
//
// Sink called by the framework just after the last context is popped off a
// document.
//----------------------------------------------------------------------------

STDAPI CSaenaruTextService::OnUninitDocumentMgr(ITfDocumentMgr *pDocMgr)
{
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// OnSetFocus
//
// Sink called by the framework when focus changes from one document to
// another.  Either document may be NULL, meaning previously there was no
// focus document, or now no document holds the input focus.
//----------------------------------------------------------------------------

STDAPI CSaenaruTextService::OnSetFocus(ITfDocumentMgr *pDocMgrFocus, ITfDocumentMgr *pDocMgrPrevFocus)
{
    // we'll track edit changes in the focus document, the only place we start compositions
    _InitTextEditSink(pDocMgrFocus);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// OnPushContext
//
// Sink called by the framework when a context is pushed.
//----------------------------------------------------------------------------

STDAPI CSaenaruTextService::OnPushContext(ITfContext *pContext)
{
#if 0
    // ignore new contexts that appear while were uninitializing
    if (!_fCleaningUp)
    {
        _InitCleanupContextSink(pContext);
        _InitContextCompartment(pContext);
    }

    return S_OK;
#endif
    return E_NOTIMPL;
}

//+---------------------------------------------------------------------------
//
// OnPopContext
//
// Sink called by the framework when a context is popped.
//----------------------------------------------------------------------------

STDAPI CSaenaruTextService::OnPopContext(ITfContext *pContext)
{
#if 0
    _UninitCleanupContextSink(pContext);
    _UninitCompartment(pContext);

    return S_OK;
#endif
    return E_NOTIMPL;
}

//+---------------------------------------------------------------------------
//
// _InitThreadMgrSink
//
// Advise our sink.
//----------------------------------------------------------------------------

BOOL CSaenaruTextService::_InitThreadMgrSink()
{
    return AdviseSink(_pThreadMgr, (ITfThreadMgrEventSink *)this,
                      IID_ITfThreadMgrEventSink, &_dwThreadMgrEventSinkCookie);
}

//+---------------------------------------------------------------------------
//
// _UninitThreadMgrSink
//
// Unadvise our sink.
//----------------------------------------------------------------------------

void CSaenaruTextService::_UninitThreadMgrSink()
{
    UnadviseSink(_pThreadMgr, &_dwThreadMgrEventSinkCookie);
}

/*
 * vim: et sts=4 sw=4
 */
