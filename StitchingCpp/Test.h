#include "header.h"
#include "plugin/Stitching_Grid.h"

class Test {
public:
    static void TestFunc(std::vector<std::string>  args) {
        std::vector<std::string>  params{
                "D:\\wkp\\testpic\\中文路径\\DK-13-16-15"
                , "out.tif"
                , "0"
                , "0"
                , "4"
                , "3"
                , "0.9"
                , "0.2"
        };

        std::vector<std::string>  files{
                "D:\\wkp\\testpic\\中文路径\\DK-13-16-15\\CL_001.tif"
                , "D:\\wkp\\testpic\\中文路径\\DK-13-16-15\\CL_002.tif"
                , "D:\\wkp\\testpic\\中文路径\\DK-13-16-15\\CL_003.tif"
                , "D:\\wkp\\testpic\\中文路径\\DK-13-16-15\\CL_004.tif"
                , "D:\\wkp\\testpic\\中文路径\\DK-13-16-15\\CL_005.tif"
                , "D:\\wkp\\testpic\\中文路径\\DK-13-16-15\\CL_006.tif"
                , "D:\\wkp\\testpic\\中文路径\\DK-13-16-15\\CL_007.tif"
                , "D:\\wkp\\testpic\\中文路径\\DK-13-16-15\\CL_008.tif"
                , "D:\\wkp\\testpic\\中文路径\\DK-13-16-15\\CL_009.tif"
                , "D:\\wkp\\testpic\\中文路径\\DK-13-16-15\\CL_010.tif"
                , "D:\\wkp\\testpic\\中文路径\\DK-13-16-15\\CL_011.tif"
                , "D:\\wkp\\testpic\\中文路径\\DK-13-16-15\\CL_012.tif"
        };

        // init log
        LOGINFO("load start\n");

        Stitching_Grid sg;
        sg.run(params, files);

        LOGINFO("load success");
    }
};