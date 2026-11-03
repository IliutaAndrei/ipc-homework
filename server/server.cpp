#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <array>
#include <fstream>
#include <filesystem>
#include <stdexcept>

constexpr unsigned short SERVER_PORT = 5000;
constexpr std::size_t BUFFER_SIZE = 4096;
constexpr std::size_t FILENAME_MAX_SIZE = 255;

using boost::asio::ip::tcp;

int main()
{
	try
	{
		boost::asio::io_context io_context;

		tcp::acceptor acceptor(
			io_context,
			tcp::endpoint(tcp::v4(), SERVER_PORT)
		);

		std::filesystem::create_directories("received_files");

		std::cout << "Server listening on port " << SERVER_PORT << '\n';

		for (;;)
		{
			tcp::socket socket(io_context);

			try
			{
				acceptor.accept(socket);
				std::cout << "Client connected!\n";

				boost::asio::streambuf filename_buffer(FILENAME_MAX_SIZE + 1);

				boost::asio::read_until(
					socket,
					filename_buffer,
					'\n'
				);

				std::istream input(&filename_buffer);

				std::string filename;
				std::getline(input, filename);

				std::filesystem::path file_path(filename);

				if (filename.empty() ||
					filename.size() > FILENAME_MAX_SIZE ||
					filename.find('\r') != std::string::npos ||
					file_path != file_path.filename() ||
					filename == "." ||
					filename == "..")
				{
					throw std::runtime_error("Invalid filename received");
				}

				std::filesystem::path output_path =
					std::filesystem::path("received_files") / file_path;

				std::ofstream fileout(output_path, std::ios::binary);

				if (!fileout.is_open())
				{
					throw std::runtime_error("Could not create output file");
				}

				if (filename_buffer.size() > 0)
				{
					fileout << &filename_buffer;

					if (!fileout)
					{
						throw std::runtime_error("Error writing initial file data");
					}
				}
				std::array<char, BUFFER_SIZE> buffer;

				while (true)
				{
					boost::system::error_code ec;

					std::size_t bytes_received = socket.read_some(
						boost::asio::buffer(buffer),
						ec
					);

					if (bytes_received > 0)
					{
						fileout.write(
							buffer.data(),
							static_cast<std::streamsize>(bytes_received)
						);

						if (!fileout)
						{
							throw std::runtime_error("Error writing received file");
						}

						std::cout << "Received " << bytes_received << " bytes\n";
					}

					if (ec == boost::asio::error::eof)
					{
						fileout.close();

						if (!fileout)
						{
							throw std::runtime_error("Error closing received file");
						}

						std::cout << "File transfer finished: " << filename << '\n';
						break;
					}

					if (ec)
					{
						throw boost::system::system_error(ec);
					}
				}
			} catch (const std::exception& e)
			{
				std::cerr << "Client error: " << e.what() << '\n';
			}
		}
	} catch (const std::exception& e)
	{
		std::cerr << "Server error: "
			<< e.what() << '\n';
		return 1;
	}
}