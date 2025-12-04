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

	//	// One more check to see if triangles were generated
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

	//// Collect all solids first
	//for (TopExp_Explorer solidExp(model, TopAbs_SOLID); solidExp.More(); solidExp.Next()) {
	//	solids.Append(solidExp.Current());
	//}

	//// Collect all shells
	//for (TopExp_Explorer shellExp(model, TopAbs_SHELL); shellExp.More(); shellExp.Next()) {
	//	shells.Append(shellExp.Current());
	//}

	//// Process solids
	//for (TopTools_ListIteratorOfListOfShape it(solids); it.More(); it.Next()) {
	//	const TopoDS_Shape& solid = it.Value();

	//	if (CanShapeBeMeshed(solid, deflection)) {
	//		builder.Add(meshableParts, solid);
	//	}
	//	else {
	//		// Solid failed, can decompose into parts
	//		TopoDS_Compound solidMeshable, solidNonMeshable;
	//		SeparateBySolidsAndShells(solid, solidMeshable, solidNonMeshable, deflection);

	//		// Merge the results
	//		MergeCompounds(solidMeshable, meshableParts);
	//		MergeCompounds(solidNonMeshable, nonMeshableParts);
	//	}
	//}

	//// Process shells
	//for (TopTools_ListIteratorOfListOfShape it(shells); it.More(); it.Next()) {
	//	const TopoDS_Shape& shell = it.Value();

	//	if (CanShapeBeMeshed(shell, deflection)) {
	//		builder.Add(meshableParts, shell);
	//	}
	//	else {
	//		// Shell failed, can decompose into faces
	//		TopoDS_Compound shellMeshable, shellNonMeshable;
	//		SeparateByFaces(shell, shellMeshable, shellNonMeshable, deflection);

	//		MergeCompounds(shellMeshable, meshableParts);
	//		MergeCompounds(shellNonMeshable, nonMeshableParts);
	//	}
	//}

	//// Process remaining faces
	//ProcessFreeFaces(model, meshableParts, nonMeshableParts, deflection);

}
