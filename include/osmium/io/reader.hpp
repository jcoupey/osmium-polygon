#ifndef OSMIUM_IO_READER_HPP
#define OSMIUM_IO_READER_HPP

/*

This file is part of Osmium (http://osmcode.org/libosmium).

Copyright 2013-2016 Jochen Topf <jochen@topf.org> and others (see README).

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

#include <cerrno>
#include <cstdlib>
#include <fcntl.h>
#include <future>
#include <memory>
#include <string>
#include <system_error>
#include <thread>
#include <utility>

#ifndef _WIN32
# include <sys/wait.h>
#endif

#ifndef _MSC_VER
# include <unistd.h>
#endif

#include <osmium/io/compression.hpp>
#include <osmium/io/detail/input_format.hpp>
#include <osmium/io/detail/read_thread.hpp>
#include <osmium/io/detail/read_write.hpp>
#include <osmium/io/detail/queue_util.hpp>
#include <osmium/io/error.hpp>
#include <osmium/io/file.hpp>
#include <osmium/io/header.hpp>
#include <osmium/memory/buffer.hpp>
#include <osmium/osm/entity_bits.hpp>
#include <osmium/thread/util.hpp>
#include <osmium/util/config.hpp>

namespace osmium {

    namespace io {

        namespace detail {

            inline size_t get_input_queue_size() noexcept {
                size_t n = osmium::config::get_max_queue_size("INPUT", 20);
                return n > 2 ? n : 2;
            }

            inline size_t get_osmdata_queue_size() noexcept {
                size_t n = osmium::config::get_max_queue_size("OSMDATA", 20);
                return n > 2 ? n : 2;
            }

        } // namespace detail

        /**
         * This is the user-facing interface for reading OSM files. Instantiate
         * an object of this class with a file name or osmium::io::File object
         * and then call read() on it in a loop until it returns an invalid
         * Buffer.
         */
        class Reader {

            osmium::io::File m_file;
            osmium::osm_entity_bits::type m_read_which_entities;

            enum class status {
                okay   = 0, // normal reading
                error  = 1, // some error occurred while reading
                closed = 2, // close() called successfully after eof
                eof    = 3  // eof of file was reached without error
            } m_status;

            int m_childpid;

            detail::future_string_queue_type m_input_queue;

            std::unique_ptr<osmium::io::Decompressor> m_decompressor;

            osmium::io::detail::ReadThreadManager m_read_thread_manager;

            detail::future_buffer_queue_type m_osmdata_queue;
            detail::queue_wrapper<osmium::memory::Buffer> m_osmdata_queue_wrapper;

            std::future<osmium::io::Header> m_header_future;
            osmium::io::Header m_header;

            osmium::thread::thread_handler m_thread;

            // This function will run in a separate thread.
            static void parser_thread(const osmium::io::File& file,
                                      detail::future_string_queue_type& input_queue,
                                      detail::future_buffer_queue_type& osmdata_queue,
                                      std::promise<osmium::io::Header>&& header_promise,
                                      osmium::osm_entity_bits::type read_which_entities) {
                std::promise<osmium::io::Header> promise = std::move(header_promise);
                auto creator = detail::ParserFactory::instance().get_creator_function(file);
                auto parser = creator(input_queue, osmdata_queue, promise, read_which_entities);
                parser->parse();
            }

#ifndef _WIN32
            /**
             * Fork and execute the given command in the child.
             * A pipe is created between the child and the parent.
             * The child writes to the pipe, the parent reads from it.
             * This function never returns in the child.
             *
             * @param command Command to execute in the child.
             * @param filename Filename to give to command as argument.
             * @returns File descriptor of pipe in the parent.
             * @throws std::system_error if a system call fails.
             */
            static int execute(const std::string& command, const std::string& filename, int* childpid) {
                int pipefd[2];
                if (pipe(pipefd) < 0) {
                    throw std::system_error(errno, std::system_category(), "opening pipe failed");
                }
                pid_t pid = fork();
                if (pid < 0) {
                    throw std::system_error(errno, std::system_category(), "fork failed");
                }
                if (pid == 0) { // child
                    // close all file descriptors except one end of the pipe
                    for (int i = 0; i < 32; ++i) {
                        if (i != pipefd[1]) {
                            ::close(i);
                        }
                    }
                    if (dup2(pipefd[1], 1) < 0) { // put end of pipe as stdout/stdin
                        exit(1);
                    }

                    ::open("/dev/null", O_RDONLY); // stdin
                    ::open("/dev/null", O_WRONLY); // stderr
                    // hack: -g switches off globbing in curl which allows [] to be used in file names
                    // this is important for XAPI URLs
                    // in theory this execute() function could be used for other commands, but it is
                    // only used for curl at the moment, so this is okay.
                    if (::execlp(command.c_str(), command.c_str(), "-g", filename.c_str(), nullptr) < 0) {
                        exit(1);
                    }
                }
                // parent
                *childpid = pid;
                ::close(pipefd[1]);
                return pipefd[0];
            }
#endif

            /**
             * Open File for reading. Handles URLs or normal files. URLs
             * are opened by executing the "curl" program (which must be installed)
             * and reading from its output.
             *
             * @returns File descriptor of open file or pipe.
             * @throws std::system_error if a system call fails.
             */
            static int open_input_file_or_url(const std::string& filename, int* childpid) {
                std::string protocol = filename.substr(0, filename.find_first_of(':'));
                if (protocol == "http" || protocol == "https" || protocol == "ftp" || protocol == "file") {
#ifndef _WIN32
                    return execute("curl", filename, childpid);
#else
                    throw io_error("Reading OSM files from the network currently not supported on Windows.");
#endif
                } else {
                    return osmium::io::detail::open_for_reading(filename);
                }
            }

        public:

            /**
             * Create new Reader object.
             *
             * @param file The file we want to open.
             * @param read_which_entities Which OSM entities (nodes, ways, relations, and/or changesets)
             *                            should be read from the input file. It can speed the read up
             *                            significantly if objects that are not needed anyway are not
             *                            parsed.
             */
            explicit Reader(const osmium::io::File& file, osmium::osm_entity_bits::type read_which_entities = osmium::osm_entity_bits::all) :
                m_file(file.check()),
                m_read_which_entities(read_which_entities),
                m_status(status::okay),
                m_childpid(0),
                m_input_queue(detail::get_input_queue_size(), "raw_input"),
                m_decompressor(m_file.buffer() ?
                    osmium::io::CompressionFactory::instance().create_decompressor(file.compression(), m_file.buffer(), m_file.buffer_size()) :
                    osmium::io::CompressionFactory::instance().create_decompressor(file.compression(), open_input_file_or_url(m_file.filename(), &m_childpid))),
                m_read_thread_manager(*m_decompressor, m_input_queue),
                m_osmdata_queue(detail::get_osmdata_queue_size(), "parser_results"),
                m_osmdata_queue_wrapper(m_osmdata_queue),
                m_header_future(),
                m_header(),
                m_thread() {
                std::promise<osmium::io::Header> header_promise;
                m_header_future = header_promise.get_future();
                m_thread = osmium::thread::thread_handler{parser_thread, std::ref(m_file), std::ref(m_input_queue), std::ref(m_osmdata_queue), std::move(header_promise), read_which_entities};
            }

            explicit Reader(const std::string& filename, osmium::osm_entity_bits::type read_types = osmium::osm_entity_bits::all) :
                Reader(osmium::io::File(filename), read_types) {
            }

            explicit Reader(const char* filename, osmium::osm_entity_bits::type read_types = osmium::osm_entity_bits::all) :
                Reader(osmium::io::File(filename), read_types) {
            }

            Reader(const Reader&) = delete;
            Reader& operator=(const Reader&) = delete;

            Reader(Reader&&) = default;
            Reader& operator=(Reader&&) = default;

            ~Reader() noexcept {
                try {
                    close();
                } catch (...) {
                    // Ignore any exceptions because destructor must not throw.
                }
            }

            /**
             * Close down the Reader. A call to this is optional, because the
             * destructor of Reader will also call this. But if you don't call
             * this function first, you might miss an exception, because the
             * destructor is not allowed to throw.
             *
             * @throws Some form of osmium::io_error when there is a problem.
             */
            void close() {
                m_status = status::closed;

                m_read_thread_manager.stop();

                m_osmdata_queue_wrapper.drain();

                try {
                    m_read_thread_manager.close();
                } catch (...) {
                    // Ignore any exceptions.
                }

#ifndef _WIN32
                if (m_childpid) {
                    int status;
                    pid_t pid = ::waitpid(m_childpid, &status, 0);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
                    if (pid < 0 || !WIFEXITED(status) || WEXITSTATUS(status) != 0) {
                        throw std::system_error(errno, std::system_category(), "subprocess returned error");
                    }
#pragma GCC diagnostic pop
                    m_childpid = 0;
                }
#endif
            }

            /**
             * Get the header data from the file.
             *
             * @returns Header.
             * @throws Some form of osmium::io_error if there is an error.
             */
            osmium::io::Header header() {
                if (m_status == status::error) {
                    throw io_error("Can not get header from reader when in status 'error'");
                }

                try {
                    if (m_header_future.valid()) {
                        m_header = m_header_future.get();
                        if (m_read_which_entities == osmium::osm_entity_bits::nothing) {
                            m_status = status::eof;
                        }
                    }
                } catch (...) {
                    close();
                    m_status = status::error;
                    throw;
                }
                return m_header;
            }

            /**
             * Reads the next buffer from the input. An invalid buffer signals
             * end-of-file. After end-of-file all read() calls will return an
             * invalid buffer. An invalid buffer is also always returned if
             * osmium::osm_entity_bits::nothing was set when the Reader was
             * constructed.
             *
             * @returns Buffer.
             * @throws Some form of osmium::io_error if there is an error.
             */
            osmium::memory::Buffer read() {
                osmium::memory::Buffer buffer;

                if (m_status != status::okay ||
                    m_read_which_entities == osmium::osm_entity_bits::nothing) {
                    throw io_error("Can not read from reader when in status 'closed', 'eof', or 'error'");
                }

                try {
                    // m_input_format.read() can return an invalid buffer to signal EOF,
                    // or a valid buffer with or without data. A valid buffer
                    // without data is not an error, it just means we have to
                    // keep getting the next buffer until there is one with data.
                    while (true) {
                        buffer = m_osmdata_queue_wrapper.pop();
                        if (detail::at_end_of_data(buffer)) {
                            m_status = status::eof;
                            m_read_thread_manager.close();
                            return buffer;
                        }
                        if (buffer.committed() > 0) {
                            return buffer;
                        }
                    }
                } catch (...) {
                    close();
                    m_status = status::error;
                    throw;
                }
            }

            /**
             * Has the end of file been reached? This is set after the last
             * data has been read. It is also set by calling close().
             */
            bool eof() const {
                return m_status == status::eof || m_status == status::closed;
            }

        }; // class Reader

        /**
         * Read contents of the given file into a buffer in one go. Takes
         * the same arguments as any of the Reader constructors.
         *
         * The buffer can take up quite a lot of memory, so don't do this
         * unless you are working with small OSM files and/or have lots of
         * RAM.
         */
        template <typename... TArgs>
        osmium::memory::Buffer read_file(TArgs&&... args) {
            osmium::memory::Buffer buffer(1024*1024, osmium::memory::Buffer::auto_grow::yes);

            Reader reader(std::forward<TArgs>(args)...);
            while (osmium::memory::Buffer read_buffer = reader.read()) {
                buffer.add_buffer(read_buffer);
                buffer.commit();
            }

            return buffer;
        }

    } // namespace io

} // namespace osmium

#endif // OSMIUM_IO_READER_HPP