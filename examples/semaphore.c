#include <windows.h>
#include <stdio.h>

#define MAX_SEM_COUNT 10
#define THREADCOUNT 12

HANDLE ghSemaphore;

DWORD WINAPI ThreadProc( LPVOID );

int main( void )
{
    HANDLE aThread[THREADCOUNT];
    DWORD ThreadID;
    int i;

    // Create a semaphore with initial and max counts of MAX_SEM_COUNT
    // Создание семафора с начальным и максимальным значениями счетчика равными MAX_SEM_COUNT

    ghSemaphore = CreateSemaphore(
        NULL,           // default security attributes // аттрибуты безопасности по умолчанию
        MAX_SEM_COUNT,  // initial count               // начальное значение счетчика
        MAX_SEM_COUNT,  // maximum count               // максимаьлное значение счетчика
        NULL);          // unnamed semaphore           // безымянный семафор

    if (ghSemaphore == NULL)
    {
        printf("CreateSemaphore error: %d\n", GetLastError());
        return 1;
    }

    // Create worker threads
    // Создаем потоки

    for( i=0; i < THREADCOUNT; i++ )
    {
        aThread[i] = CreateThread(
                     NULL,       // default security attributes   // аттрибуты безопасности по умолчанию
                     0,          // default stack size            // размер стека по умолчанию
                     (LPTHREAD_START_ROUTINE) ThreadProc,
                     NULL,       // no thread function arguments  // отсутствуют аргументы для функции-тела потока
                     0,          // default creation flags        // флаги создания потока по умолчанию
                     &ThreadID); // receive thread identifier     // идентификатор потока

        if( aThread[i] == NULL )
        {
            printf("CreateThread error: %d\n", GetLastError());
            return 1;
        }
    }

    // Wait for all threads to terminate
    // Ждем завершения всех потоков

    WaitForMultipleObjects(THREADCOUNT, aThread, TRUE, INFINITE);

    // Close thread and semaphore handles
    // Закрываем потоки и дескриптор семафора

    for( i=0; i < THREADCOUNT; i++ )
        CloseHandle(aThread[i]);

    CloseHandle(ghSemaphore);

    return 0;
}

DWORD WINAPI ThreadProc( LPVOID lpParam )
{

    // lpParam not used in this example
    // аргумент (параметр) lpParam в этом примере не используется
    UNREFERENCED_PARAMETER(lpParam);

    DWORD dwWaitResult;
    BOOL bContinue=TRUE;

    while(bContinue)
    {
        // Try to enter the semaphore gate.
        // Пытаемся захватить семафор

        dwWaitResult = WaitForSingleObject(
            ghSemaphore,   // handle to semaphore           // дескриптор семафора
            0L);           // zero-second time-out interval // нулевое время ожидания

        switch (dwWaitResult)
        {
            // The semaphore object was signaled.
            // Семафор удалось захватить успешно
            case WAIT_OBJECT_0:
                // TODO: Perform task
                // Здесь должен быть код, решающий требуемую задачу
                printf("Thread %d: wait succeeded\n", GetCurrentThreadId());
                bContinue=FALSE;            

                // Simulate thread spending time on task
                // Симулируем работу потока
                Sleep(5);

                // Release the semaphore when task is finished
                // Отпускаем семафор по завершении задачи

                if (!ReleaseSemaphore(
                        ghSemaphore,  // handle to semaphore              // дескриптор семафора
                        1,            // increase count by one            // увеличиваем значение счетчика на единицу
                        NULL) )       // not interested in previous count // игнорируем предыдущее значение счетчика
                {
                    printf("ReleaseSemaphore error: %d\n", GetLastError());
                }
                break;

            // The semaphore was nonsignaled, so a time-out occurred.
            // Семафор не удалось захватить в течении выбранного времени ожидания
            case WAIT_TIMEOUT:
                printf("Thread %d: wait timed out\n", GetCurrentThreadId());
                break;
        }
    }
    return TRUE;
}
