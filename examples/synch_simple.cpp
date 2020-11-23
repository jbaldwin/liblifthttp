#include <lift/lift.hpp>

#include <iostream>

int main()
{
    {
        lift::request request{"http://www.example.com"};
        std::cout << "Requesting http://www.example.com" << std::endl;
        const auto& response = request.perform();
        std::cout << response.data() << std::endl;
    }

    {
        lift::request request{"http://www.google.com"};
        std::cout << "Requesting http://www.google.com" << std::endl;
        const auto& response = request.perform();
        std::cout << response.data() << std::endl;

        for (const auto& header : response.headers())
        {
            std::cout << header.name() << ": " << header.value() << "\n";
        }
    }

    return 0;
}
