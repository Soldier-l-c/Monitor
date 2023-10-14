// ConsoleApplication1.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <thread>
#pragma comment(lib, "pdh.lib")

bool PdhCollectQueryMon(const wchar_t* counter_path, int32_t counter_time, double& avg);

int main()
{
    int32_t counter_time{ 60 };

    double processor{ 0 }, disk{ 0 };

    std::thread pro([&] {PdhCollectQueryMon(L"\\Processor(_Total)\\% Processor Time", counter_time, processor); });
    std::thread dis([&] {PdhCollectQueryMon(L"\\PhysicalDisk(* E:)\\% Disk Time", counter_time, disk); });

    Sleep((counter_time+1) * 1000);

    pro.join();
    dis.join();

    std::cout << "pro:[" << processor << "], disk:[" << disk << "]\n";
    system("pause");
    return 0;
}

bool PdhCollectQueryMon(const wchar_t* counter_path, int32_t counter_time, double& avg)
{
    HQUERY hQuery = NULL;
    HCOUNTER hCounter = NULL;
    PDH_STATUS status = ERROR_SUCCESS;
    DWORD dwFormat = PDH_FMT_DOUBLE;
    PDH_FMT_COUNTERVALUE ItemBuffer;

    // Opens the log file to write performance data
    status = PdhOpenQuery(nullptr, 0, &hQuery);

    do
    {
        if (ERROR_SUCCESS != status)
        {
            wprintf(L"PdhOpenQuery failed with 0x%x\n", status);
            break;
        }

        // Add the same counter used when writing the log file.
        status = PdhAddCounter(hQuery, counter_path, 0, &hCounter);

        if (ERROR_SUCCESS != status)
        {
            wprintf(L"PdhAddCounter failed with 0x%x\n", status);
            break;
        }

        Sleep(1000);

        // Read a performance data record.
        status = PdhCollectQueryData(hQuery);

        if (ERROR_SUCCESS != status)
        {
            wprintf(L"PdhCollectQueryData failed with 0x%x\n", status);
            break;
        }

        double total_counter{ 0 };
        int32_t sssss = counter_time;
        while (ERROR_SUCCESS == status && counter_time-- > 0)
        {
            // Read the next record
            status = PdhCollectQueryData(hQuery);

            if (ERROR_SUCCESS == status)
            {
                // Format the performance data record.
                status = PdhGetFormattedCounterValue(hCounter,
                    dwFormat,
                    (LPDWORD)NULL,
                    &ItemBuffer);

                if (ERROR_SUCCESS != status)
                {
                    wprintf(L"PdhGetFormattedCounterValue failed with 0x%x.\n", status);
                    break;
                }

                wprintf(L"counter path:[%s] Formatted counter value = %.20g\n", counter_path, ItemBuffer.doubleValue);
                total_counter += ItemBuffer.doubleValue;
            }
            else
            {
                if (PDH_NO_MORE_DATA != status)
                {
                    wprintf(L"PdhCollectQueryData failed with 0x%x\n", status);
                }
            }
            Sleep(1000);
        }
        avg = total_counter / sssss;

    } while (false);

    if (hQuery)
    {
        PdhCloseQuery(hQuery);
    }
    else
    {
        return false;
    }

    return true;
}
