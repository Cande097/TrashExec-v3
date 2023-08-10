#pragma once
#include <string>
#include <Windows.h>

namespace win32
{
    class File
    {
    private:

        struct
        {
            std::string fileName = nullptr;

            HANDLE fileHandle = INVALID_HANDLE_VALUE;
        };

    public:

        ~File() = default;

        File(const std::string& fileName)
            : fileName(fileName) {}

    private:

        bool PrepareFile(DWORD flag)
        {
            this->fileHandle = CreateFileA(this->fileName.c_str(),
                GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                flag, FILE_ATTRIBUTE_NORMAL, nullptr);

            if (this->fileHandle == INVALID_HANDLE_VALUE) //Validate.
            {
                return false; //Return false since handle is invalid.
            }

            return true; //Return true since all went as it expected.
        }

    public:

        bool Read(std::string& content)
        {
            if (!this->PrepareFile(OPEN_ALWAYS)) //Prepare the file.
            {
                return false; //Return false since handle is invalid.
            }

            const auto fileSize = GetFileSize( //Retrieve file size.
                this->fileHandle, nullptr);

            auto buffer = new char[fileSize]; //Setup the buffer ptr.

            auto bytesRead = DWORD(); //Define readed bytes size obj.

            const auto result = ReadFile(this->fileHandle, buffer,
                fileSize, &bytesRead, nullptr);

            if (result != 0) //Check if the readed results were valid.
            {
                content.assign(buffer, fileSize); //Assign buf to str.
            }

            delete[] buffer; //Delete the assigned buffer for content.

            return result != 0; //Check if the file results are valid.
        }

        bool Write(const std::string& content)
        {
            if (!this->PrepareFile(CREATE_ALWAYS)) //Prepare the file.
            {
                return false; //Return false since handle is invalid.
            }

            auto bytesWritten = DWORD(); //Define written bytes object.

            const auto result = WriteFile(this->fileHandle,
                content.c_str(), content.size(), &bytesWritten, NULL);


            return result != 0; //Check if the file results are valid.
        }


        void Close()
        {
            CloseHandle(this->fileHandle); //Close created file handle.
        }
    };

    inline bool DirectoryExists(const std::string& path)
    {
        DWORD attribs = ::GetFileAttributesA(path.data());
        if (attribs == INVALID_FILE_ATTRIBUTES)
        {
            return false;
        }

        return (attribs & FILE_ATTRIBUTE_DIRECTORY);
    }

    inline bool CreateDirectory(const std::string& path, bool createAlways = false)
    {
        return CreateDirectoryA(path.c_str(), createAlways) != 0; //Do.
    }

}
