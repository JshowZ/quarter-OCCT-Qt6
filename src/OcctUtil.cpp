#include "OcctUtil.h"
#include <BRepMesh_IncrementalMesh.hxx>

OcctUtil::OcctUtil()
{
}

OcctUtil::~OcctUtil()
{
}

bool CanShapeBeMeshed(const TopoDS_Shape& shape, double deflection) {
	//try {
	//	BRepMesh_IncrementalMesh mesher(shape, deflection);
	//	mesher.Perform();

	//	if (!mesher.IsDone()) return false;

	//	// 进一步验证：检查是否有三角剖分
	//	return HasValidTriangulation(shape);
	//}
	//catch (...) {
	//	return false;
	//}
	return true;
}

void OcctUtil::SeparateBySolidsAndShells(const TopoDS_Shape& model, TopoDS_Compound& meshableParts, TopoDS_Compound& nonMeshableParts, double deflection)
{

	//BRep_Builder builder;
	//builder.MakeCompound(meshableParts);
	//builder.MakeCompound(nonMeshableParts);

	//TopTools_ListOfShape solids, shells;

	//// 收集所有实体
	//for (TopExp_Explorer solidExp(model, TopAbs_SOLID); solidExp.More(); solidExp.Next()) {
	//	solids.Append(solidExp.Current());
	//}

	//// 收集所有壳
	//for (TopExp_Explorer shellExp(model, TopAbs_SHELL); shellExp.More(); shellExp.Next()) {
	//	shells.Append(shellExp.Current());
	//}

	//// 测试实体
	//for (TopTools_ListIteratorOfListOfShape it(solids); it.More(); it.Next()) {
	//	const TopoDS_Shape& solid = it.Value();

	//	if (CanShapeBeMeshed(solid, deflection)) {
	//		builder.Add(meshableParts, solid);
	//	}
	//	else {
	//		// 实体失败，尝试分解为壳
	//		TopoDS_Compound solidMeshable, solidNonMeshable;
	//		SeparateBySolidsAndShells(solid, solidMeshable, solidNonMeshable, deflection);

	//		// 将结果合并到输出
	//		MergeCompounds(solidMeshable, meshableParts);
	//		MergeCompounds(solidNonMeshable, nonMeshableParts);
	//	}
	//}

	//// 测试壳
	//for (TopTools_ListIteratorOfListOfShape it(shells); it.More(); it.Next()) {
	//	const TopoDS_Shape& shell = it.Value();

	//	if (CanShapeBeMeshed(shell, deflection)) {
	//		builder.Add(meshableParts, shell);
	//	}
	//	else {
	//		// 壳失败，尝试分解为面
	//		TopoDS_Compound shellMeshable, shellNonMeshable;
	//		SeparateByFaces(shell, shellMeshable, shellNonMeshable, deflection);

	//		MergeCompounds(shellMeshable, meshableParts);
	//		MergeCompounds(shellNonMeshable, nonMeshableParts);
	//	}
	//}

	//// 处理剩余的自由面
	//ProcessFreeFaces(model, meshableParts, nonMeshableParts, deflection);

}
