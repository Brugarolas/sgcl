//------------------------------------------------------------------------------
// SGCL: Smart Garbage Collection Library
// Copyright (c) 2022-2024 Sebastian Nibisz
// SPDX-License-Identifier: Zlib
//------------------------------------------------------------------------------
#pragma once

#include "block_allocator.h"
#include "object_allocator.h"

namespace sgcl {
    namespace Priv {
        struct Small_object_allocator_base : Object_allocator {
            Small_object_allocator_base(Block_allocator& ba, Pointer_pool_base& pa, Page*& pb, std::atomic_flag& lock) noexcept
                : _block_allocator(ba)
                , _pointer_pool(pa)
                , _pages_buffer(pb)
                , _lock(lock) {
            }

            ~Small_object_allocator_base() noexcept override {
                while (!_pointer_pool.is_empty()) {
                    auto ptr = _pointer_pool.alloc();
                    auto index = _current_page->index_of(ptr);
                    _current_page->states()[index].store(State::Unused, std::memory_order_relaxed);
                }
                std::atomic_thread_fence(std::memory_order_release);
            }

            void* alloc(size_t = 0) {
                if  (_pointer_pool.is_empty()) {
                    while (_lock.test_and_set(std::memory_order_acquire));
                    auto page = _pages_buffer;
                    if (page) {
                        _pages_buffer = page->next_empty;
                    }
                    _lock.clear(std::memory_order_release);
                    if (page) {
                        _pointer_pool.fill(page);
                        page->on_empty_list.store(false, std::memory_order_release);
                    } else {
                        page = _alloc_page();
                        _pointer_pool.fill((void*)(page->data));
                        page->next = pages.load(std::memory_order_acquire);
                        while(!pages.compare_exchange_weak(page->next, page, std::memory_order_release, std::memory_order_relaxed));
                    }
                    _current_page = page;
                }
                assert(!_pointer_pool.is_empty());
                return (void*)_pointer_pool.alloc();
            }

        private:
            Block_allocator& _block_allocator;
            Pointer_pool_base& _pointer_pool;
            Page*& _pages_buffer;
            std::atomic_flag& _lock;
            Page* _current_page = {nullptr};

            virtual Page* _create_page_parameters(Data_page*) = 0;

            Page* _alloc_page() {
                auto data = _block_allocator.alloc();
                auto page = _create_page_parameters(data);
                data->page = page;
                return page;
            }

        protected:
            static void _remove_empty(Page*& pages, Page*& empty_pages) noexcept {
                auto page = pages;
                Page* prev = nullptr;
                while(page) {
                    auto next = page->next_empty;
                    auto states = page->states();
                    auto object_count = page->metadata->object_count;
                    auto unused_count = 0u;
                    for (unsigned i = 0; i < object_count; ++i) {
                        auto state = states[i].load(std::memory_order_relaxed);
                        if (state == State::Unused) {
                            ++unused_count;
                        }
                    }
                    if (unused_count == object_count) {
                        page->next_empty = empty_pages;
                        empty_pages = page;
                        if (!prev) {
                            pages = next;
                        } else {
                            prev->next_empty = next;
                        }
                    } else {
                        prev = page;
                    }
                    page = next;
                }
                std::atomic_thread_fence(std::memory_order_release);
            }

            static void _free(Page* pages) noexcept {
                Page* page = pages;
                Data_page* empty = nullptr;
                while(page) {
                    Data_page* data = (Data_page*)(page->data - sizeof(void*));
                    data->block = page->block;
                    page->is_used = false;
                    data->next = empty;
                    empty = data;
                    page = page->next_empty;
                }
                Block_allocator::free(empty);
            }

            static void _free(Page* pages, Page*& pages_buffer, std::atomic_flag& lock) noexcept {
                Page* empty_pages = nullptr;
                for (int i = 0; i < 2 ; ++i) {
                    _remove_empty(pages, empty_pages);
                    while (lock.test_and_set(std::memory_order_acquire));
                    std::swap(pages, pages_buffer);
                    lock.clear(std::memory_order_release);
                }
                if (pages) {
                    auto last = pages;
                    while(last->next_empty) {
                        last = last->next_empty;
                    }
                    while (lock.test_and_set(std::memory_order_acquire));
                    last->next_empty = pages_buffer;
                    pages_buffer = pages;
                    lock.clear(std::memory_order_release);
                }
                if (empty_pages) {
                    _free(empty_pages);
                }
            }
        };
    }
}
