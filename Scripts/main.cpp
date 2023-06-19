#include <iostream>
#include <vector>

// Function to extract a cell from a 1D depth image vector
std::vector<float> extractCell(const std::vector<float>& image, int width, int topLeftX, int topLeftY, int bottomRightX, int bottomRightY)
{
	std::vector<float> cell;
	int cellWidth = bottomRightX - topLeftX + 1;
	int cellHeight = bottomRightY - topLeftY + 1;

	for (int y = topLeftY; y <= bottomRightY; ++y)
	{
		for (int x = topLeftX; x <= bottomRightX; ++x)
		{
			int index = y * width + x;  // Calculate the index in the 1D vector
			cell.push_back(image[index]);
		}
	}

	return cell;
}

int main()
{
	std::vector<float> depthImage = { 1.2, 2.3, 3.4, 4.5, 5.6, 6.7, 7.8, 8.9, 9.0 };
	int width = 3;  // Width of the depth image

	int topLeftX = 0;
	int topLeftY = 0;
	int bottomRightX = 1;
	int bottomRightY = 1;

	std::vector<float> cellData = extractCell(depthImage, width, topLeftX, topLeftY, bottomRightX, bottomRightY);

	// Display the extracted cell data
	std::cout << "Cell Data: ";
	for (const auto& value : cellData)
	{
		std::cout << value << " ";
	}
	std::cout << std::endl;
	
	std::cin.get();
	return 0;
}
