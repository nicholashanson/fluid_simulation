#include <gtest/gtest.h>

#include <geometry/polygon.hpp>

TEST( GeometryTest, IncreasePolygonTreeDepth ) {
	geometry::polygon_tree<std::size_t> root{};
	root.index = 0;
	root.height = 0;
	geometry::increase_polygon_tree_depth( &root );
	EXPECT_EQ( root.height, 1 );
}

TEST( GeometryTest, IncreasePolygonTreeDepth_Child ) {
	geometry::polygon_tree<std::size_t> root{};
	root.index = 0;
	root.height = 0;
	geometry::polygon_tree<std::size_t> child{};
	child.index = 1;
	child.height = 1;
	child.parent = &root;
	root.children.insert( &child );
	geometry::increase_polygon_tree_depth( &root );
	EXPECT_EQ( root.height, 1 );
	EXPECT_EQ( child.height, 2 );
}

TEST( GeometryTest, IncreasePolygonTreeDepth_Children ) {
	geometry::polygon_tree<std::size_t> root{};
	root.index = 0;
	root.height = 0;
	geometry::polygon_tree<std::size_t> child1{};
	child1.index = 1;
	child1.height = 1;
	child1.parent = &root;
	root.children.insert( &child1 );
	geometry::polygon_tree<std::size_t> child2{};
	child2.index = 2;
	child2.height = 1;
	child2.parent = &root;
	root.children.insert( &child2 );
	geometry::increase_polygon_tree_depth( &root );
	EXPECT_EQ( root.height, 1 );
	EXPECT_EQ( child1.height, 2 );
	EXPECT_EQ( child2.height, 2 );
}