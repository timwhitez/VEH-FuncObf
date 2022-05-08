#include "windows.h"
#include "stdio.h"

void WINAPI my_sleep(DWORD ms) {
    printf("[?] Hooked Sleep Function Called!\n");
    printf("Sleeping for: %d\n", ms);
}

LPVOID SleepA;
LPVOID JMPA;

LONG NTAPI FirstVectExcepHandler(PEXCEPTION_POINTERS pExcepInfo)
{
    // 捕获PAGE_GUARD_VIOLATION异常
    if (pExcepInfo->ExceptionRecord->ExceptionCode == STATUS_GUARD_PAGE_VIOLATION) {
        // 如果EIP/RIP是目标函数，就修改EIP/RIP来跳转到Hook函数
        if (pExcepInfo->ContextRecord->Rip == (uintptr_t)SleepA) {
            printf("HOOKED\n");
            printf("SleepA\n");
            pExcepInfo->ContextRecord->Rip = (uintptr_t)(JMPA);
        }

        //pExcepInfo->ContextRecord->EFlags |= 0x100; // 将TF置1，下一条指令执行完后会触发SINGLE_STEP异常
        return EXCEPTION_CONTINUE_EXECUTION; // 继续下一条指令
    }
    //else if (pExcepInfo->ExceptionRecord->ExceptionCode == STATUS_SINGLE_STEP) {// 捕获SINGLE_STEP异常
    //    DWORD old;
    //    VirtualProtect(SleepA, 1, PAGE_EXECUTE_READ | PAGE_GUARD, &old); // 重新加上PAGE_GUARD标志，因为每次触发过异常后系统都会清除它

    //    return EXCEPTION_CONTINUE_EXECUTION; // 继续下一条指令
    //}

    return EXCEPTION_CONTINUE_SEARCH; // 如果不是PAGE_GUARD也不是SINGLE_STEP，就拒绝处理
}

//void set(LPVOID a) {
//    SleepA = a;
//}
void test() {
    AddVectoredExceptionHandler(1, &FirstVectExcepHandler);
}

typedef int(WINAPI* checkapi)(
    _In_opt_ HWND hWnd,
    _In_opt_ LPCSTR lpText,
    _In_opt_ LPCSTR lpCaption,
    _In_ UINT uType);


int Mmain() {
    test();
    DWORD old;
    checkapi NEWA = (checkapi)Sleep;

    JMPA = MessageBoxA;
    VirtualProtect(Sleep, 1, PAGE_EXECUTE_READ | PAGE_GUARD, &old);
    SleepA = Sleep;
    NEWA(NULL, "Test123", "Test", MB_OK);
    VirtualProtect(Sleep, 1, PAGE_EXECUTE_READ | PAGE_GUARD, &old);
    Sleep(NULL, "Test123", "Test", MB_OK);
    Sleep(5000);
    //Sleep(30000);
    return 0;
}

void main() {
    Mmain();
}
