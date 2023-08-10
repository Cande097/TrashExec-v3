#include <string>
#include <windows.h>

class File
{
private:

    std::string fileName = nullptr;

public:

    ~File() = default;

    File(const std::string& fileName) : fileName(fileName) {}

public:

    bool read(std::string& content)
    {
        auto fileHandle = HANDLE(); //Define the file handle obj.

        if (!this->getFileHandle(OPEN_ALWAYS, fileHandle))
        {
            return false; //Return false since handle is invalid.
        }

        const auto fileSize = GetFileSize( //Retrieve file size.
            fileHandle, nullptr);

        auto buffer = new char[fileSize]; //Setup the buffer ptr.

        auto bytesRead = DWORD(); //Define readed bytes size obj.

        const auto result = ReadFile(fileHandle, buffer,
            fileSize, &bytesRead, nullptr);

        CloseHandle(fileHandle); //Close the created file handle.

        if (result != 0) //Check if the readed results were valid.
        {
            content.assign(buffer, fileSize); //Assign buf to str.
        }

        delete[] buffer; //Delete the assigned buffer for content.

        return result != 0; //Check if the file results are valid.
    }

    bool write(const std::string& content)
    {
        auto fileHandle = HANDLE(); //Define the file handle obj.

        if (!this->getFileHandle(CREATE_ALWAYS, fileHandle))
        {
            return false; //Return false since handle is invalid.
        }

        auto bytesWritten = DWORD(); //Define written bytes object.

        const auto result = WriteFile(fileHandle,
            content.c_str(), content.size(), &bytesWritten, NULL);

        CloseHandle(fileHandle); //Close the created file handle.

        return result != 0; //Check if the file results are valid.
    }

    bool createDirectory(const std::string& path)
    {
        return CreateDirectoryA(path.c_str(), nullptr) != 0; //Do.
    }

private:

    bool getFileHandle(DWORD flag, HANDLE& handle)
    {
        handle = CreateFileA(this->fileName.c_str(), 0xc0000000,
            0, nullptr, flag, FILE_ATTRIBUTE_NORMAL, nullptr);

        if (handle == INVALID_HANDLE_VALUE) //Validate the handle.
        {
            return false; //Return false the handle is not valid.
        }

        return true; //Return true since all went as we expected.
    }
};
