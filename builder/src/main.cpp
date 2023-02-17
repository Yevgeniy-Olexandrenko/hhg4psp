#include "Config.h"
#include "Builder.h"
#include "Image.h"

std::string m_games[] =
{
	//	wide screen
//	"gnw_mmouse",
//	"gnw_chef",
//	"gnw_octopus",
///	"gnw_fire",
//	"gnw_pchute",
///	"gnw_popeye",
///	"gnw_egg",

	//	silver/gold
	"gnw_ball",
	"gnw_fires",
	"gnw_flagman",
	"gnw_vermin",
	"gnw_helmet",
	"gnw_judge",

	//	ussr clones
//	"nupogodi",
///	"exospace",
///	"merrycook",
///	"spacebridge",
//	"autoslalom"
};


//void png_test()
//{
//	//Image::Format format = Image::Format1111.WithColorDithering(true).WithAlphaDithering(false);
//	Image::Format format{ 4, 4, 4, 4, Image::DitheringJarvisJudiceAndNinke, Image::DitheringJarvisJudiceAndNinke };
//
//	Image gray("gray.png", format);
//	gray.Save("gray_.png");
//
//	Image gradient("gradient.png", format);
//	gradient.Save("gradient_.png");
//
//	//Image cube("cube.png", format);
//	//cube.Save("cube_.png");
//
//	//Image cube_bw("cube_bw.png", format);
//	//cube_bw.Save("cube_bw_.png");
//
//	//Image lenna("lenna.png", format);
//	//lenna.Save("lenna_.png");
//
//	//Image small_fg("small_fg.png", format);
//	//small_fg.Save("small_fg_.png");
//
//	Image test(256, 256, false);
//	for (size_t y = 0; y < test.GetH(); ++y)
//	{
//		for (size_t x = 0; x < test.GetW(); ++x)
//		{
//			Image::Pixel::Channel i = Image::Pixel::Channel(std::sqrtf(float(x) * float(y)));
//			test.SetPixel(x, y, Image::Pixel(i, i, i));
//		}
//	}
//	test.Quantize(format);
//	test.Save("test.png");
//
//	//int ext = 50;
//	//Image::Format format1{ 5, 6, 5, 0, false, false };
//	//Image::Format format2{ 4, 4, 4, 4, false, false };
//	//Image test1("small_fg.png", format1);
//	//Image test2("small_fg.png", format2);
//	//Image result(test1.GetW(), test1.GetH(), true);
//	//result.CopyPixels(test1, 0, 0, 0, 0, test1.GetW(), test1.GetH());
//	//result.CopyPixels(test2, 122 - ext, 60 - ext, 122 - ext, 60 - ext, 233 + 2 * ext, 151 + 2 * ext);
//	//result.Save("chief.png");
//
//	Image alpha_gradient("alpha_gradient.png", format);
//	alpha_gradient.Save("alpha_gradient_.png");
//}

int main()
{
//	png_test();

	for (const auto & game : m_games)
	{
		std::string configFilePath = "games/" + game + "/" + game + ".yaml";
		printf("Build: %s\n", configFilePath.c_str());

		Builder builder(configFilePath);
		bool done = builder.Build();

		printf("Build: %s\n\n", (done ? "DONE" : "FAILED"));
	}
}
