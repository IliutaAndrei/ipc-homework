#include <iostream>
#include <boost/asio.hpp>
#include <fstream>
#include <vector>
#include <iterator>
#include <filesystem>

constexpr const char* LOCALHOST = "127.0.0.1";
constexpr unsigned short SERVER_PORT = 5000;
constexpr unsigned short FILENAME_MAX_SIZE = 255;

using boost::asio::ip::tcp;


int main(int argc, char* argv[])
{
	try
	{
		if (argc != 2)
		{
			std::cerr << "Usage: client <filename>" << std::endl;
			return 1;
		}

		std::filesystem::path file_path(argv[1]);
		std::string filename = file_path.filename().string();

		if (filename.empty() ||
			filename.size() > FILENAME_MAX_SIZE ||
			filename.find('\n') != std::string::npos)
		{
			std::cerr << "Invalid filename\n";
			return 1;
		}

		std::ifstream filein(argv[1], std::ios::binary);

		if (!filein.is_open())
		{
			std::cerr << "Could not open: " << argv[1] << "\n";
			return 1;
		}

		std::vector<char> file_data(
			(std::istreambuf_iterator<char>(filein)),
			std::istreambuf_iterator<char>()
		);

		if (filein.bad())
		{
			std::cerr << "Error reading file\n";
			return 1;
		}

		boost::asio::io_context io_context;
		tcp::endpoint endpoint(
			boost::asio::ip::make_address(LOCALHOST),
			SERVER_PORT
		);

		tcp::socket socket(io_context);
		socket.connect(endpoint);
		std::cout << "Connected to server\n";

		std::string header = filename + "\n";
		boost::asio::write(socket, boost::asio::buffer(header));
		boost::asio::write(socket, boost::asio::buffer(file_data));

		socket.shutdown(tcp::socket::shutdown_send);
		std::cout << "File sent successfully\n";
		

	} catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
}
