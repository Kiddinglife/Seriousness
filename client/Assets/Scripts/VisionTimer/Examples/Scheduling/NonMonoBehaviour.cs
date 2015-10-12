/////////////////////////////////////////////////////////////////////////////////
//
//	NonMonoBehaviour.cs
//
//	description:	this is just a basic non-monobehaviour for the scheduling example
//
/////////////////////////////////////////////////////////////////////////////////

using UnityEngine;

public class NonMonoBehaviour
{

	public void Test()
	{

		TestComponent tc = (TestComponent)GameObject.Find("ExternalGameObject").GetComponent("TestComponent");
		tc.Test("Hello World!");

	}

}
