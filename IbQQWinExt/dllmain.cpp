#include "pch.h"
#include <string>
#include <boost/di.hpp>
#include "helper.hpp"

using namespace std;
namespace di = boost::di;

namespace QQ {
    const wchar Path[] = LR"(C:\Program Files (x86)\Tencent\QQ\Bin\QQ.exe)";

    namespace Modules {
        class IM : public Module {
        public:
            IM() : Module(*makeModule::Load(L"Common.dll")) {
                DebugOutput(to_wstring((uint32_t)base));
            }
            ~IM() {
                Free();
            }
        };

        class MsgMgr : public Module {
        public:
            MsgMgr() : Module(*makeModule::Load(L"MsgMgr.dll")) {
                DebugOutput(to_wstring((uint32_t)base));
            }
            ~MsgMgr() {
                Free();
            }
        };
    }

    class BypassMsgRevoke {
        static int __stdcall HandleNotifyGroupMessageRevokedDetour(int nGroupType, int a5, void* a6) {
            int a1;
            char* a2;
            int a3;
            __asm {
                mov a1, ecx
                mov a2, edi
                mov a3, esi
            }

            int r;
            __asm {
                push a6
                push a5
                push nGroupType
                mov ecx, a1
                mov edi, a2
                mov esi, a3
                call HandleNotifyGroupMessageRevoked
                mov r, eax
            }
            return r;
        }
        static inline decltype(HandleNotifyGroupMessageRevokedDetour)* HandleNotifyGroupMessageRevoked;
    public:
        BypassMsgRevoke(Modules::IM& IM, Modules::MsgMgr& MsgMgr) {
            HandleNotifyGroupMessageRevoked = MsgMgr.base.offset(0x6AEDF);
            //IbDetourAttach(&HandleNotifyGroupMessageRevoked, HandleNotifyGroupMessageRevokedDetour);
        }
    };


};

class QQExt {
public:
    QQExt() {
        //static auto BypassMsgRevoke = di::make_injector().create<QQ::BypassMsgRevoke>();
    }
};

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    if (DetourIsHelperProcess())
        return TRUE;
    optional<QQExt> qqext;
    
    wstring path = *makeModule::CurrentProcess()->GetPath();
    DebugOutput(L"Loaded("s + to_wstring(ul_reason_for_call) + L") in: " + path);

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        DetourRestoreAfterWith();
        if (!_wcsicmp(path.c_str(), QQ::Path))
            qqext = QQExt();
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}