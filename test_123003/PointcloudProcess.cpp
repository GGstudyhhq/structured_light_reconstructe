#include "PointcloudProcess.h"

const char* pnts3D_pcdfilename = "../result/points3d.pcd";
const char* pnts3D_filtered_pcdfilename = "../result/points3d_filtered.pcd";
 
void savepointcloud(cv::Mat& pnts)
{
	pcl::PointCloud<pcl::PointXYZ> cloud;
	cv::Mat filterpnts3D(2000, 2000, CV_32FC1);

	cloud.width = pnts.cols;
	cloud.height = 1;
	cloud.points.resize(cloud.width * cloud.height);
  
	float *pnts3D_row1 = pnts.ptr<float>(0);	
	float *pnts3D_row2 = pnts.ptr<float>(1);
	float *pnts3D_row3 = pnts.ptr<float>(2);
	float *pnts3D_row4 = pnts.ptr<float>(3);
	for(int i = 0; i < pnts.cols; i++)
	{
		float pnts3D_data4 = *(pnts3D_row4 + i);
		float pnts3D_data1 = *(pnts3D_row1 + i) / pnts3D_data4;
		float pnts3D_data2 = *(pnts3D_row2 + i) / pnts3D_data4;
		float pnts3D_data3 = *(pnts3D_row3 + i) / pnts3D_data4;
    
		// pnts3D_data3 = 1400;
		// std::cout << "cloud.points.size = " << cloud.points.size() <<endl;
		// filterpnts3D.at<float>(i/2000, i%2000) = pnts3D_data3/1000.0f;
    
		if(i < cloud.points.size())
		{
			//cloud.points[i].x = pnts3D_data1/1000.0f;
			//cloud.points[i].y = pnts3D_data2/1000.0f;
			//cloud.points[i].z = pnts3D_data3/1000.0f;

			cloud.points[i].x = pnts3D_data1/1000.0f;
			cloud.points[i].y = pnts3D_data2/1000.0f;
			cloud.points[i].z = pnts3D_data3/1000.0f;
		}
	}
  
	// cv::medianBlur(filterpnts3D, filterpnts3D, 3);
	// for(int i=0; i < cloud.points.size(); i++)
	// {
	//    cloud.points[i].z = filterpnts3D.at<float>(i/2000, i%2000);
	// }
  
	pcl::io::savePCDFileASCII(pnts3D_pcdfilename, cloud);
  
#if 0
	pcl::PointCloud<pcl::PointXYZ>::Ptr viewcloud;
	pcl::io::loadPCDFile<pcl::PointXYZ>(pnts3D_pcdfilename, *viewcloud);

	pcl::visualization::CloudViewer viewer("viewer");
	viewer.showCloud(viewcloud);
	while(!viewer.wasStopped());
#endif
}
 
void filterpointcloud(void)
{
	pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);
	pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_filtered(new pcl::PointCloud<pcl::PointXYZ>);
	pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_res(new pcl::PointCloud<pcl::PointXYZ>);

	pcl::PCDReader reader;
	reader.read(pnts3D_pcdfilename, *cloud);
  
	std::cout << "Pointcloud before filtering: " << cloud->width*cloud->height << "data points" <<endl;

#if 0
	/**********************VoxelGrid************************************/
	pcl::VoxelGrid< pcl::PointXYZ > sor;
	sor.setInputCloud(cloud);
	sor.setLeafSize(0.0008f, 0.0008f, 0.0015f);
	sor.filter(*cloud_filtered);
  
	std::cout << "Pointcloud after filtering: " << cloud_filtered->width*cloud_filtered->height << "data points" <<endl;

	pcl::PCDWriter writer;
	writer.write(pnts3D_filtered_pcdfilename, *cloud_filtered);
#endif

	/****************************Radius filter*************************/
	pcl::RadiusOutlierRemoval<pcl::PointXYZ> outrem;
	outrem.setInputCloud(cloud);
	outrem.setRadiusSearch(0.01);
	outrem.setMinNeighborsInRadius(30);//30
	outrem.filter(*cloud_filtered);
  
	std::cout << "Pointcloud after filtering: " << cloud_filtered->width*cloud_filtered->height << "data points" <<endl;

	pcl::io::savePCDFileASCII("../result/points3d_res.pcd", *cloud_filtered);  
}

// 泊松重建
void poissonreconstruction(void)
{
	pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);
	pcl::PointCloud<pcl::PointXYZ>::Ptr mls_cloud(new pcl::PointCloud<pcl::PointXYZ>);
	pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_filtered(new pcl::PointCloud<pcl::PointXYZ>);
	pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_passfilter(new pcl::PointCloud<pcl::PointXYZ>);

	pcl::PCDReader reader;
	reader.read(pnts3D_pcdfilename, *cloud);
  
	/**********************mls移动最小二乘滤波*****************************************************/
	pcl::search::KdTree<pcl::PointXYZ>::Ptr tree_mls(new pcl::search::KdTree<pcl::PointXYZ>);
	pcl::PointCloud<pcl::PointNormal> mls_points;
	pcl::MovingLeastSquares<pcl::PointXYZ, pcl::PointNormal> mls;
	mls.setComputeNormals(true);
	mls.setInputCloud(cloud);
	mls.setSearchMethod(tree_mls);
	mls.setSearchRadius(0.003);
  
	mls.process(mls_points);
  
	pcl::io::savePCDFileASCII("../result/points3d_res.pcd", mls_points);  

	reader.read("../result/points3d_res.pcd", *mls_cloud);

	cout << "===> " << mls_cloud->points.size() << endl;
  
	/***************************fliter********************************/
#if 0
	//直通滤波
	pcl::PassThrough<pcl::PointXYZ> pass;

	pass.setInputCloud(mls_cloud);
	pass.setFilterFieldName("z");
	pass.setFilterLimits(1.45,1.9);
	pass.setFilterLimitsNegative(false);
	pass.filter(*cloud_passfilter);
#endif

	cout << "===> " << cloud_passfilter->points.size() << endl;
  
#if 0
	//半径滤波
	pcl::RadiusOutlierRemoval<pcl::PointXYZ> outrem;
	outrem.setInputCloud(cloud_passfilter);
	outrem.setRadiusSearch(0.01);
	outrem.setMinNeighborsInRadius(30);
	outrem.filter(*cloud_filtered);
#endif
  
	cout << "===> " << cloud_filtered->points.size() << endl;

	/*******************************mls***************************************/  
	pcl::NormalEstimation<pcl::PointXYZ, pcl::Normal> n;
	pcl::PointCloud<pcl::Normal>::Ptr normals(new pcl::PointCloud<pcl::Normal>);
	pcl::search::KdTree<pcl::PointXYZ>::Ptr tree(new pcl::search::KdTree<pcl::PointXYZ>);
	//tree->setInputCloud(cloud_filtered);
	//n.setInputCloud(cloud_filtered);
	tree->setInputCloud(mls_cloud);
	n.setInputCloud(mls_cloud);
	n.setSearchMethod(tree);
	n.setKSearch(20);
	n.compute(*normals);

	pcl::PointCloud<pcl::PointNormal>::Ptr cloud_with_normals(new pcl::PointCloud<pcl::PointNormal>);
	//pcl::concatenateFields(*cloud_filtered, *normals, *cloud_with_normals);
	pcl::concatenateFields(*mls_cloud, *normals, *cloud_with_normals);
  
	pcl::search::KdTree<pcl::PointNormal>::Ptr tree2(new pcl::search::KdTree<pcl::PointNormal>);
	tree2->setInputCloud(cloud_with_normals);

	/**********************poisson reconstruction***********************/
	pcl::Poisson<pcl::PointNormal> poisson;
	poisson.setConfidence(false);
	poisson.setDegree(2);
	poisson.setDepth(8);
	poisson.setIsoDivide(8);
	poisson.setManifold(true);
	poisson.setOutputPolygons(true);
	poisson.setSamplesPerNode(3.0);
	poisson.setScale(1.25);
	poisson.setSolverDivide(8);
	poisson.setSearchMethod(tree2);
	poisson.setInputCloud(cloud_with_normals);
	pcl::PolygonMesh mesh;
	poisson.performReconstruction(mesh);
  
	boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer(new pcl::visualization::PCLVisualizer("3D viewer"));
	viewer->setBackgroundColor(0,0,0);
	viewer->addPolygonMesh(mesh, "mesh");
	// viewer->setRepresentationToPointsForAllActors(); // 以点的形式显示
	viewer->setRepresentationToSurfaceForAllActors();  //以面的形式显示
	// viewer->setRepresentationToWireframeForAllActors(); // 以网格的形式显示
	viewer->addCoordinateSystem(0.0);
	viewer->initCameraParameters();
	while(!viewer->wasStopped())
	{
		viewer->spinOnce(100);
	}
}


