/////////////////////////////////////////////////////////////////////////////////
//
//	TestComponent.cs
//
//	description:	this is just a basic monobehaviour for the scheduling example
//
/////////////////////////////////////////////////////////////////////////////////

using UnityEngine;

public class TestComponent : MonoBehaviour
{

	Color m_StatusColor = Color.white;
	string m_StatusString = "";
	
	void Update ()
	{

		m_StatusColor = Color.Lerp(m_StatusColor, new Color(1, 1, 1, 0), Time.deltaTime * 2.75f);

	}

	void OnGUI()
	{

		GUI.color = m_StatusColor;
		GUI.Label(new Rect(Screen.width - 190, Screen.height - (Screen.height - 95), 400, 50), m_StatusString);

	}

	public void Test(string s)
	{
		m_StatusColor = Color.yellow;
		m_StatusString = s;
	}

}
