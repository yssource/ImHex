#pragma once

#include <hex.hpp>

#include <cmath>
#include <concepts>
#include <functional>
#include <list>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace hex::prv {

    class Provider {
    public:
        constexpr static size_t PageSize = 0x1000'0000;

        Provider() {
            this->m_patches.emplace_back();
        }

        virtual ~Provider() = default;

        virtual bool isAvailable() = 0;
        virtual bool isReadable() = 0;
        virtual bool isWritable() = 0;

        virtual size_t read(u64 offset, void *buffer, size_t size) {
            size_t readSize = this->readRaw(offset, buffer, size);

            if (readSize == 0)
                return 0;

            for (u64 i = 0; i < readSize; i++) {
                if (this->m_patches.back().contains(offset + i))
                    reinterpret_cast<u8*>(buffer)[i] = this->m_patches.back()[offset + i];

                for (const auto &overlay : this->m_overlays) {
                    auto byte = overlay(offset + i, reinterpret_cast<u8*>(buffer)[i]);
                    if (byte.has_value())
                        reinterpret_cast<u8*>(buffer)[i] = byte.value();
                }
            }

            return readSize;
        }
        virtual size_t write(u64 offset, const void *buffer, size_t size) {
            size_t writeSize = this->writeRaw(offset, buffer, size);

            if (writeSize == 0)
                return 0;

            this->m_patches.push_back(this->m_patches.back());

            for (u64 i = 0; i < size; i++)
                this->m_patches.back()[offset + i] = reinterpret_cast<const u8*>(buffer)[i];

            return writeSize;
        }

        virtual size_t readRaw(u64 offset, void *buffer, size_t size) = 0;
        virtual size_t writeRaw(u64 offset, const void *buffer, size_t size) = 0;
        virtual size_t getActualSize() = 0;

        std::map<u64, u8>& getPatches() { return this->m_patches.back(); }
        void applyPatches() {
            for (auto &[patchAddress, patch] : this->m_patches.back())
                this->writeRaw(patchAddress, &patch, 1);
        }

        auto addOverlay(std::function<std::optional<u8>(u64, u8)> &&function) {
            return this->m_overlays.insert(this->m_overlays.end(), function);
        }

        void removeOverlay(std::list<std::function<std::optional<u8>(u64, u8)>>::iterator iterator) {
            this->m_overlays.erase(iterator);
        }

        u32 getPageCount() { return std::ceil(this->getActualSize() / double(PageSize)); }
        u32 getCurrentPage() const { return this->m_currPage; }
        void setCurrentPage(u32 page) { if (page < getPageCount()) this->m_currPage = page; }

        virtual size_t getBaseAddress() {
            return PageSize * this->m_currPage;
        }

        virtual size_t getSize() {
            return std::min(this->getActualSize() - PageSize * this->m_currPage, PageSize);
        }

        virtual std::optional<u32> getPageOfAddress(u64 address) {
            u32 page = std::floor(address / double(PageSize));

            if (page >= this->getPageCount())
                return { };

            return page;
        }

        virtual std::vector<std::pair<std::string, std::string>> getDataInformation() = 0;

    protected:
        u32 m_currPage = 0;

        std::vector<std::map<u64, u8>> m_patches;
        std::list<std::function<std::optional<u8>(u64, u8)>> m_overlays;
    };

}