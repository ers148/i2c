//! @file Application.hpp
//! @author Aleksei Drovnenkov
//! @date Jun 08, 2023

#ifndef BOARD_STM32F0_APPLICATION_HPP_
#define BOARD_STM32F0_APPLICATION_HPP_

#include <array>
#include <chrono>

using namespace std::chrono_literals;

template <typename Board>
class Application {
	using I2c = typename Board::I2c;
	using Clock = typename Board::Clock;

	static constexpr auto& kAddress = Board::kAddress;
	static constexpr auto& kRequest = Board::kRequest;
	static constexpr auto& kResponse = Board::kResponse;
	static constexpr auto kResponseSize = kResponse.size();

public:
	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;

	//! @brief Constructor
	//! @param[in] i2cInstance - i2c driver instance
	Application(I2c &i2cInstance)
		: i2c{i2cInstance}
	{
	}

	//! @brief Application main loop
	void run()
	{
		for(;;) {
			Clock::delay(std::chrono::milliseconds(990));
			response.fill(0);
			result = i2c.exchange(kAddress, kRequest.data(), kRequest.size(), response.data(), kResponseSize);
			if (result) {
				Clock::delay(std::chrono::milliseconds(10));
				result = response == kResponse;
			} else {
				i2c.reset();
			}
		}
	}

private:
	I2c &i2c;
	volatile bool result;
	std::array<uint8_t, kResponseSize> response;
};

#endif // BOARD_STM32F0_APPLICATION_HPP_
