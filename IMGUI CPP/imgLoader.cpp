#include "imgLoader.h"

#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <dirent.h>
#include <unordered_map>
#include <algorithm>
#include <cstring>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "Debug.h"

const size_t SECTOR = 2048;

struct ImgEntry
{
    std::string name;          // trimmed name (no nulls)
    std::vector<uint8_t> data; // raw stored data (already sector-padded size)
};

// ---------------- helpers ----------------

static inline std::string trim_cstring_field(const char* cstr, size_t maxlen)
{
    // ensure we stop at first NUL
    size_t i = 0;
    for (; i < maxlen && cstr[i] != '\0'; ++i) {}
    return std::string(cstr, cstr + i);
}

static inline std::vector<uint8_t> read_file(const std::string& path)
{
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f.is_open()) return {};
    std::streampos sz = f.tellg();
    if (sz <= 0) return {};
    std::vector<uint8_t> buf(static_cast<size_t>(sz));
    f.seekg(0, std::ios::beg);
    f.read(reinterpret_cast<char*>(buf.data()), buf.size());
    return buf;
}

static inline size_t get_file_size_ifstream(const std::string& path)
{
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f.is_open()) return 0;
    std::streampos sz = f.tellg();
    return sz > 0 ? static_cast<size_t>(sz) : 0;
}

static inline std::unordered_map<std::string, size_t> read_sizes(const std::string& path)
{
    std::unordered_map<std::string, size_t> map;
    std::ifstream f(path);
    if (!f.is_open()) return map;
    std::string name;
    size_t size;
    while (f >> name >> size) map[name] = size;
    return map;
}

static inline void write_sizes(const std::string& path, const std::unordered_map<std::string, size_t>& map)
{
    std::ofstream f(path, std::ios::trunc);
    if (!f.is_open()) return;
    for (auto& p : map) f << p.first << " " << p.second << "\n";
}

// ---------------- read existing IMG ----------------
// reads VER2 img into vector of ImgEntry (data size = sectors * SECTOR)
static inline bool read_gta3_img(const std::string& imgPath, std::vector<ImgEntry>& outEntries)
{
    std::ifstream in(imgPath, std::ios::binary);
    if (!in.is_open()) return false;

    char header[4];
    in.read(header, 4);
    if (in.gcount() != 4 || std::strncmp(header, "VER2", 4) != 0)
    {
        in.close();
        return false;
    }

    uint32_t numEntries = 0;
    in.read(reinterpret_cast<char*>(&numEntries), 4);
    if (in.fail())
    {
        in.close();
        return false;
    }

    struct DirRaw
    {
        uint32_t offset;
        uint16_t sizeSectors;
        uint16_t dummy;
        char name[24];
    };
    std::vector<DirRaw> dir(numEntries);
    for (uint32_t i = 0; i < numEntries; ++i)
    {
        in.read(reinterpret_cast<char*>(&dir[i]), 32);
        if (in.gcount() != 32 && in.eof())
        {
            in.close();
            return false;
        }
    }

    outEntries.clear();
    for (uint32_t i = 0; i < numEntries; ++i)
    {
        ImgEntry e;
        e.name = trim_cstring_field(dir[i].name, 24);
        size_t byteSize = static_cast<size_t>(dir[i].sizeSectors) * SECTOR;
        if (byteSize == 0) { e.data.clear(); }
        else
        {
            e.data.resize(byteSize);
            in.seekg(static_cast<std::streamoff>(dir[i].offset) * static_cast<std::streamoff>(SECTOR), std::ios::beg);
            in.read(reinterpret_cast<char*>(e.data.data()), byteSize);
            if (in.gcount() != static_cast<std::streamsize>(byteSize))
            {
                // read failed (corrupted or unexpected EOF) -> abort
                in.close();
                return false;
            }
        }
        outEntries.push_back(std::move(e));
    }

    in.close();
    return true;
}

// ---------------- write IMG safely (tmp + rename) ----------------
static inline bool write_gta3_img_atomic(const std::string& imgPath, const std::vector<ImgEntry>& entries)
{
    std::string tmpPath = imgPath + ".tmp";

    // open tmp file with POSIX to be able to fsync
    int fd = open(tmpPath.c_str(), O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (fd < 0) return false;
    FILE* fout = fdopen(fd, "wb");
    if (!fout)
    {
        close(fd);
        unlink(tmpPath.c_str());
        return false;
    }
    std::ofstream out;
    // we'll use FILE* for buffered write then fsync via fd
    // write header
    if (fwrite("VER2", 1, 4, fout) != 4)
    {
        fclose(fout);
        unlink(tmpPath.c_str());
        return false;
    }
    uint32_t num = static_cast<uint32_t>(entries.size());
    if (fwrite(&num, sizeof(num), 1, fout) != 1)
    {
        fclose(fout);
        unlink(tmpPath.c_str());
        return false;
    }

    // reserve directory space
    std::vector<uint8_t> emptyDir(entries.size() * 32, 0);
    if (entries.size() && fwrite(emptyDir.data(), 1, emptyDir.size(), fout) != emptyDir.size())
    {
        fclose(fout);
        unlink(tmpPath.c_str());
        return false;
    }

    // write files aligned
    std::vector<uint32_t> offsets(entries.size());
    std::vector<uint16_t> sizes(entries.size());
    for (size_t i = 0; i < entries.size(); ++i)
    {
        // align to sector
        long cur = ftell(fout);
        if (cur < 0)
        {
            fclose(fout);
            unlink(tmpPath.c_str());
            return false;
        }
        size_t pad = static_cast<size_t>(cur) % SECTOR ? (SECTOR - (static_cast<size_t>(cur) % SECTOR)) : 0;
        if (pad)
        {
            std::vector<char> z(pad, 0);
            if (fwrite(z.data(), 1, z.size(), fout) != z.size())
            {
                fclose(fout);
                unlink(tmpPath.c_str());
                return false;
            }
        }
        long posAfterPad = ftell(fout);
        offsets[i] = static_cast<uint32_t>(posAfterPad / SECTOR);

        // write data
        const auto& d = entries[i].data;
        if (!d.empty())
        {
            if (fwrite(d.data(), 1, d.size(), fout) != d.size())
            {
                fclose(fout);
                unlink(tmpPath.c_str());
                return false;
            }
            size_t rem = d.size() % SECTOR;
            if (rem)
            {
                std::vector<char> z(SECTOR - rem, 0);
                if (fwrite(z.data(), 1, z.size(), fout) != z.size())
                {
                    fclose(fout);
                    unlink(tmpPath.c_str());
                    return false;
                }
            }
            sizes[i] = static_cast<uint16_t>((d.size() + SECTOR - 1) / SECTOR);
        }
        else
        {
            sizes[i] = 0;
        }
    }

    // flush and fsync
    fflush(fout);
    int r = fsync(fd);
    if (r != 0)
    {
        fclose(fout);
        unlink(tmpPath.c_str());
        return false;
    }

    // write directory (seek and rewrite)
    if (fseek(fout, 8, SEEK_SET) != 0)
    {
        fclose(fout);
        unlink(tmpPath.c_str());
        return false;
    }
    for (size_t i = 0; i < entries.size(); ++i)
    {
        uint32_t off = offsets[i];
        uint16_t s = sizes[i];
        uint16_t dummy = 0;
        char nameField[24] = { 0 };
        strncpy(nameField, entries[i].name.c_str(), 24);
        if (fwrite(&off, 4, 1, fout) != 1)
        {
            fclose(fout);
            unlink(tmpPath.c_str());
            return false;
        }
        if (fwrite(&s, 2, 1, fout) != 1)
        {
            fclose(fout);
            unlink(tmpPath.c_str());
            return false;
        }
        if (fwrite(&dummy, 2, 1, fout) != 1)
        {
            fclose(fout);
            unlink(tmpPath.c_str());
            return false;
        }
        if (fwrite(nameField, 1, 24, fout) != 24)
        {
            fclose(fout);
            unlink(tmpPath.c_str());
            return false;
        }
    }

    fflush(fout);
    r = fsync(fd);
    if (r != 0)
    {
        fclose(fout);
        unlink(tmpPath.c_str());
        return false;
    }
    fclose(fout);

    // atomic rename
    if (rename(tmpPath.c_str(), imgPath.c_str()) != 0)
    {
        unlink(tmpPath.c_str());
        return false;
    }
    return true;
}

// ---------------- core loader logic ----------------
void RunIMGLoader()
{
    std::string texdbPath = std::string(aml->GetAndroidDataPath()) + "/texdb";
    std::string modloaderFolder = texdbPath + "/modloader";
    std::string dffsPath = modloaderFolder + "/dff";
    std::string sizesPath = modloaderFolder + "/sizes.txt";
    std::string imgPath = texdbPath + "/gta3.img";

    ModData::CreateFolder(modloaderFolder);
    ModData::CreateFolder(dffsPath);

    fileLog->Log("RunIMGLoader: scanning " + dffsPath);

    // load existing sizes
    auto savedSizes = read_sizes(sizesPath);
    std::unordered_map<std::string, size_t> newSizes = savedSizes;

    // read current IMG entries to memory
    std::vector<ImgEntry> entries;
    if (!read_gta3_img(imgPath, entries))
    {
        fileLog->Log("RunIMGLoader: failed to read gta3.img (aborting)");
        return;
    }

    // list dffs directory
    DIR* dir = opendir(dffsPath.c_str());
    if (!dir)
    {
        fileLog->Log("RunIMGLoader: no modloader/dff folder");
        return;
    }

    bool anyChange = false;
    struct dirent* ent;
    while ((ent = readdir(dir)) != nullptr)
    {
        std::string fname = ent->d_name;
        if (fname.size() > 4 && fname.substr(fname.size() - 4) == ".dff")
        {
            std::string full = dffsPath + "/" + fname;
            size_t sz = get_file_size_ifstream(full);
            size_t prev = 0;
            auto itPrev = savedSizes.find(fname);
            if (itPrev != savedSizes.end()) prev = itPrev->second;

            if (prev != sz)
            {
                fileLog->Log("RunIMGLoader: changed/new DFF: " + fname);

                menuDebug->AddLine("[DFF loader] changing " + fname + "...");

                // read file bytes
                auto data = read_file(full);
                if (data.empty())
                {
                    fileLog->Log("RunIMGLoader: failed read file " + full);
                    continue;
                }

                // sector-pad the data to store exactly in IMG entries
                size_t sectors = (data.size() + SECTOR - 1) / SECTOR;
                size_t padded = sectors * SECTOR;
                data.resize(padded, 0);

                // find entry by name (case-sensitive; adjust if you want case-insensitive)
                bool found = false;
                for (auto& e : entries)
                {
                    if (e.name == fname)
                    {
                        e.data = data; // replace
                        found = true;
                        anyChange = true;
                        break;
                    }
                }

                if (!found)
                {
                    // add new entry at end
                    ImgEntry ne;
                    ne.name = fname;
                    ne.data = data;
                    entries.push_back(std::move(ne));
                    anyChange = true;
                }

                newSizes[fname] = sz;
            }
        }
    }
    closedir(dir);

    if (anyChange)
    {
        fileLog->Log("RunIMGLoader: changes detected, writing new gta3.img...");
        if (write_gta3_img_atomic(imgPath, entries))
        {
            fileLog->Log("RunIMGLoader: gta3.img updated successfully");
            menuDebug->AddLine("[DFF loader] gta3.img updated successfully");

            write_sizes(sizesPath, newSizes);
        }
        else
        {
            fileLog->Log("RunIMGLoader: failed to write new gta3.img");
            menuDebug->AddLine("[DFF loader] ~r~failed to write new gta3.img");
        }
    }
    else
    {
        fileLog->Log("RunIMGLoader: no changes");

        menuDebug->AddLine("[DFF loader] no changes found");
    }
}