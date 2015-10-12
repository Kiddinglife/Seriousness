/////////////////////////////////////////////////////////////////////////////////
//
//	SchedulingDemo.cs
//
//	description:	this class illustrates a number of common use cases for the
//					vp_Timer class. it covers methods, arguments, delegates,
//					iterations, intervals, canceling and object accessibility
//
/////////////////////////////////////////////////////////////////////////////////

using UnityEngine;


public class SchedulingDemo : MonoBehaviour
{

	// misc variables for gui and animation
	Vector3 m_CubeScale = new Vector3(0.5f, 0.5f, 0.5f);
	Color m_StatusColor = Color.white;
	string m_StatusString = "";
	public AudioClip m_SmackSound = null;


	/// <summary>
	/// here we animate the cube every frame
	/// </summary>
	void Update()
	{

		// set the position of the cube every frame, making it
		// adapting to different screen resolutions
		transform.position = (FindObjectOfType(typeof(Camera))as Camera).ScreenToWorldPoint(
												new Vector3(Screen.width - 165, Screen.height - 105, 6));

		// make the cube shrink back to its normal scale if smacked
		transform.localScale = Vector3.Lerp(transform.localScale, m_CubeScale, Time.deltaTime * 5.0f);

		// skip rendering the cube if it's super small
		GetComponent<Renderer>().enabled = (transform.localScale.x > 0.01f) ? true : false;

		// make the cube fade back to red if made yellow
		GetComponent<Renderer>().material.color = Color.Lerp(GetComponent<Renderer>().material.color, Color.red, Time.deltaTime * 3f);

	}


	/// <summary>
	/// this method contains all timer example code. NOTE: timer code
	/// does not have to be located in 'OnGUI'. it can exist anywhere
	/// </summary>
	void OnGUI()
	{

		// make the status text fade back to invisible if made yellow
		m_StatusColor = Color.Lerp(m_StatusColor, new Color(1, 1, 0, 0), Time.deltaTime * 0.5f);

		// draw the header text
		GUI.color = Color.white;
		GUILayout.Space(50);
		GUILayout.BeginHorizontal();
		GUILayout.Space(50);
		GUILayout.Label("SCHEDULING EXAMPLE\n    - Each of these examples will schedule some functionality in one (1)\n      second using different options.\n    - Please study the source code in 'Examples/Scheduling/Scheduling.cs' ...");
		GUILayout.EndHorizontal();

		// create an area for all the example buttons
		GUILayout.BeginArea(new Rect(100, 150, 400, 600));
		GUI.color = Color.white;
		GUILayout.Label("Methods, Arguments, Delegates, Iterations, Intervals & Canceling");

		// --- Example 1 ---
		if (DoButton("A simple method"))
		{
			vp_Timer.In(1.0f, DoMethod);
		}

		// --- Example 2 ---
		if (DoButton("A method with a single argument"))
		{
			vp_Timer.In(1.0f, DoMethodWithSingleArgument, 242);
		}

		// --- Example 3 ---
		if (DoButton("A method with multiple arguments"))
		{

			object[] arg = new object[3];
			arg[0] = "December";
			arg[1] = 31;
			arg[2] = 2012;
			vp_Timer.In(1.0f, DoMethodWithMultipleArguments, arg);

			// TIP: you can also create the object array in-line like this:
			//vp_Timer.In(1.0f, DoMethodWithMultipleArguments, new object[] { "December", 31, 2012 });

		}

		// --- Example 4 ---
		if (DoButton("A delegate"))
		{
			vp_Timer.In(1.0f, delegate()
			{
				SmackCube();
			});
		}

		// --- Example 5 ---
		if (DoButton("A delegate with a single argument"))
		{
			vp_Timer.In(1.0f, delegate(object o)
			{
				int i = (int)o;
				SmackCube();
				SetStatus("A delegate with a single argument ... \"" + i + "\"");
			}, 242);
		}

		// --- Example 6 ---
		if (DoButton("A delegate with multiple arguments"))
		{
			vp_Timer.In(1.0f, delegate(object o)
			{

				object[] argument = (object[])o;		// 'unpack' object into an array
				string month = (string)argument[0];		// convert the first argument to a string
				int day = (int)argument[1];				// convert the second argument to an integer
				int year = (int)argument[2];			// convert the third argument to an integer

				SmackCube();
				SetStatus("A delegate with multiple arguments ... \"" + month + " " + day + ", " + year + "\"");

			}, new object[] { "December", 31, 2012 });
		}

		// --- Example 7 ---
		if (DoButton("5 iterations of a method"))
		{
			vp_Timer.In(1.0f, SmackCube, 5);
		}

		// --- Example 8 ---
		if (DoButton("5 iterations of a method, with 0.2 sec intervals"))
		{
			vp_Timer.In(1.0f, SmackCube, 5, 0.2f);
		}

		// --- Example 9 ---
		if (DoButton("5 iterations of a delegate, canceled after 3 seconds"))
		{
			vp_Timer.Handle timer = new vp_Timer.Handle();
			vp_Timer.In(0.0f, delegate() { SmackCube(); }, 5, 1,
				timer);
			vp_Timer.In(3, delegate() { timer.Cancel(); });
		}

		GUILayout.Label("\nMethod & object accessibility:");

		// --- Example 10 ---
		if (DoButton("Running a method from a non-monobehaviour class", false))
		{
			vp_Timer.In(1, delegate()
			{
				NonMonoBehaviour test = new NonMonoBehaviour();
				test.Test();
			});

		}

		// --- Example 11 ---
		if (DoButton("Running a method from a specific external gameobject", false))
		{
			vp_Timer.In(1, delegate()
			{
				TestComponent tc = (TestComponent)GameObject.Find("ExternalGameObject").GetComponent("TestComponent");
				tc.Test("Hello World!");
			});
		}

		// --- Example 12 ---
		if (DoButton("Running a method from the first component of a certain type\nin current transform or any of its children", false))
		{
			vp_Timer.In(1, delegate()
			{
				TestComponent tc = transform.root.GetComponentInChildren<TestComponent>();
				tc.Test("Hello World!");
			});
		}

		// --- Example 13 ---
		if (DoButton("Running a method from the first component of a certain type\nin the whole Hierarchy", false))
		{
			vp_Timer.In(1, delegate()
			{
				TestComponent tc = (TestComponent)FindObjectOfType(typeof(TestComponent));
				tc.Test("Hello World!");
			});
		}

		GUILayout.EndArea();

		GUI.color = m_StatusColor;

		GUILayout.BeginArea(new Rect(Screen.width - 255, 205, 240, 600));
		GUILayout.Label(m_StatusString);
		GUILayout.EndArea();

	}


	/// <summary>
	/// target method for example 1
	/// </summary>
	void DoMethod()
	{

		SmackCube();

	}


	/// <summary>
	/// target method for example 2
	/// </summary>
	void DoMethodWithSingleArgument(object o)
	{

		int i = (int)o;
		SmackCube();
		SetStatus("A method with a single argument ... \"" + i + "\"");

	}


	/// <summary>
	/// target method for example 3
	/// </summary>
	void DoMethodWithMultipleArguments(object o)
	{

		object[] argument = (object[])o;		// 'unpack' object into an array
		string month = (string)argument[0];		// convert the first argument to a string
		int day = (int)argument[1];				// convert the second argument to an integer
		int year = (int)argument[2];			// convert the third argument to an integer

		SetStatus("A method with multiple arguments ... \"" + month + ", " + day + ", " + year + "\"");
		SmackCube();

	}

	
	/// <summary>
	/// triggers animation on the cube by color, scale and angle.
	/// 'Update' will take care of the actual animating
	/// </summary>
	public void SmackCube()
	{

		// scale and paint
		transform.localScale += new Vector3(0.6f, 0.6f, 0.6f);
		GetComponent<Renderer>().material.color = Color.yellow;

		// generate a a random rotation value for the cube. avoid values
		// close to zero (too slow)
		Vector3 rotation = new Vector3(Random.Range(50.0f, 100.0f), Random.Range(50.0f, 100.0f),
										Random.Range(50.0f, 100.0f));

		// reverse the rotation in 50% of the cases
		if(Random.value < 0.5f)	rotation.x = -rotation.x;
		if(Random.value < 0.5f)	rotation.y = -rotation.y;
		if(Random.value < 0.5f)	rotation.z = -rotation.z;

		// add the random rotation to the rigidbody of the cube
		GetComponent<Rigidbody>().maxAngularVelocity = 1000;	// enable high rotation velocities
		GetComponent<Rigidbody>().AddTorque(rotation);

		// play the 'smack' sound
		GetComponent<AudioSource>().PlayOneShot(m_SmackSound);

	}


	/// <summary>
	/// a compound control / wrapper for a regular Unity GUI button,
	/// with some very special cases for this demo
	/// </summary>
	bool DoButton(string s, bool showCube = true)
	{

		bool clicked = false;
		GUILayout.BeginHorizontal();
		GUILayout.Space(30);
		if (GUILayout.Button(s + "."))
		{
			// scale down the cube for the second group of examples
			if (!showCube)
				m_CubeScale = new Vector3(0.001f, 0.001f, 0.001f);
			else
				m_CubeScale = new Vector3(0.5f, 0.5f, 0.5f);

			SetStatus(s + " ...");
			clicked = true;
		}
		GUILayout.EndHorizontal();
		return clicked;

	}

	
	/// <summary>
	/// sets the string of the status text and makes it visible
	/// </summary>
	void SetStatus(string s)
	{

		m_StatusString = s;
		m_StatusColor = Color.yellow;

	}


}
