/////////////////////////////////////////////////////////////////////////////////
//
//	StopWatch.cs
//
//	description:	this class shows how to set up a stopwatch showing seconds
//					and hundredths. it uses a vp_Timer object that runs forever,
//					but can be stopped and paused using buttons
//
/////////////////////////////////////////////////////////////////////////////////

using UnityEngine;


class StopWatch : MonoBehaviour
{

	// variables
	public bool DrawButtons = true;
	public bool DrawDigitalWatch = true;
	public bool DrawAnalogWatch = true;
	public float FontBackMargin = 8;
	public float FontBackSizeOffset = -29;
	public Vector2 FontPosOffset = new Vector2(14.0f, 13.0f);
	public Vector2 ButtonsPosition = new Vector2(0, 0);
	public Vector2 ButtonsScale = new Vector2(80, 40);
	public Vector2 DigitalPosition = new Vector2(255, -230);
	public Vector2 AnalogPosition = new Vector2(-240, 140);
	public Vector2 BigHandPos = new Vector2(0, 0);
	public Vector2 SmallHandPos = new Vector2(0, -53);
	public Color DigitalNumbersColor = Color.yellow;

	// timer handles
	vp_Timer.Handle m_Timer = new vp_Timer.Handle();

	// images
	public Texture ImagePixel = null;
	public Texture ImageDigitalDisplay = null;
	public Texture ImageStopWatch = null;
	public Texture ImageHandBig = null;
	public Texture ImageHandSmall = null;
	public Texture ImageButtonPlay = null;
	public Texture ImageButtonPause = null;
	public Texture ImageButtonStop = null;

	// gui skin
	public GUISkin Skin = null;

	GUIStyle m_DigitalDisplayStyle = null;
	GUIStyle DigitalDisplayStyle
	{
		get
		{
			if (m_DigitalDisplayStyle == null)
				m_DigitalDisplayStyle = Skin.GetStyle("DigitalDisplay");
			return m_DigitalDisplayStyle;
		}
	}


	/// <summary>
	/// 
	/// </summary>
	void OnGUI()
	{

		if (DrawDigitalWatch)
			DrawDigital();

		if (DrawButtons)
			DrawDemoButtons();

		if (DrawAnalogWatch)
			DrawAnalog();

	}


	/// <summary>
	/// draws a play/stop and a pause/play button to set the state
	/// of the stopwatch timer
	/// </summary>
	void DrawDemoButtons()
	{

		// draw play/stop button
		if (GUI.Button(new Rect((Screen.width * 0.5f) + ButtonsPosition.x - (ButtonsScale.x / 2),
								(Screen.height * 0.5f) + ButtonsPosition.y - ButtonsScale.y, ButtonsScale.x,
								ButtonsScale.y), m_Timer.Active ? ImageButtonStop : ImageButtonPlay))
		{

			if (!m_Timer.Active)
				Run();
			else
				Stop();

		}

		// draw pause/play button
		if (!m_Timer.Active)
			GUI.enabled = false;
		if (GUI.Button(new Rect((Screen.width * 0.5f) + ButtonsPosition.x - (ButtonsScale.x / 2),
								(Screen.height * 0.5f) + ButtonsPosition.y + 5, ButtonsScale.x, ButtonsScale.y),
								m_Timer.Paused ? ImageButtonPlay : ImageButtonPause))
		{
			Pause();
		}
		GUI.enabled = true;

	}


	/// <summary>
	/// draws the digital display for the stopwatch. the vp_TimeUtility
	/// class is used to convert the vp_Timer duration into a formatted
	/// string showing minutes, seconds and hundredths delimited by ':'
	/// </summary>
	void DrawDigital()
	{

		// format time strings
		string digitalDisplayTimeString = "";

		digitalDisplayTimeString = vp_TimeUtility.TimeToString(m_Timer.Duration, false, true, true, false, true, false);

		// draw background for digital display
		Rect rect = new Rect((Screen.width * 0.5f) + DigitalPosition.x - (ImageDigitalDisplay.width * 0.5f),
							(Screen.height * 0.5f) + DigitalPosition.y - (ImageDigitalDisplay.height * 0.5f),
							(ImageDigitalDisplay.width + FontBackSizeOffset),
							(ImageDigitalDisplay.height + FontBackSizeOffset));

		GUI.color = Color.black;
		GUI.DrawTexture(new Rect(rect.x - FontBackMargin + FontPosOffset.x, rect.y - FontBackMargin + FontPosOffset.y,
						rect.width - 1 + (FontBackMargin * 2) + 1, rect.height - 1 + (FontBackMargin * 2) + 1),
						ImagePixel);

		// draw digital numbers
		GUI.color = DigitalNumbersColor;
		GUI.DrawTexture(new Rect(rect.x + 1 + FontPosOffset.x, rect.y + 2 + FontPosOffset.y, rect.width - 1, rect.height - 1),
						ImagePixel);
		GUI.color = Color.black;
		GUI.Label(new Rect(rect.x + FontPosOffset.x, rect.y + FontPosOffset.y, rect.width, rect.height), digitalDisplayTimeString,
							DigitalDisplayStyle);

		// draw frame for digital display
		GUI.color = Color.white;
		GUI.DrawTexture(new Rect(rect.x, rect.y, ImageDigitalDisplay.width, ImageDigitalDisplay.height), ImageDigitalDisplay);

	}
	

	/// <summary>
	/// draws the analog display for the stopwatch. the vp_TimeUtility
	/// class is used to convert the vp_Timer duration into angles for
	/// the clock hands
	/// </summary>
	void DrawAnalog()
	{

		// prepare for drawing analog watch
		GUI.color = Color.white;
		Vector2 stopWatchPos = new Vector2(((Screen.width * 0.5f) + AnalogPosition.x) - (ImageStopWatch.width / 2),
											((Screen.height * 0.5f) + AnalogPosition.y) - (ImageStopWatch.height / 2));
		Vector3 watchAngle = Vector3.zero;

		// determine angles for watch hands
		watchAngle.x = 0.0f;
		watchAngle.y = vp_TimeUtility.TimeToDegrees(m_Timer.Duration, false, false, false, true);	// smooth milliseconds
		watchAngle.z = vp_TimeUtility.TimeToDegrees(m_Timer.Duration, false, false, true, true);	// smooth seconds

		// draw big watch hand
		GUIUtility.RotateAroundPivot(watchAngle.z, stopWatchPos + new Vector2((ImageStopWatch.width / 2) + BigHandPos.x,
									(ImageStopWatch.height / 2) + BigHandPos.y));

		GUI.DrawTexture(new Rect(stopWatchPos.x + BigHandPos.x, stopWatchPos.y + BigHandPos.y, ImageStopWatch.width,
						ImageStopWatch.height), ImageHandBig);

		GUI.matrix = Matrix4x4.TRS(Vector3.zero, Quaternion.identity, Vector3.one);

		// draw small watch hand
		GUIUtility.RotateAroundPivot(watchAngle.y, stopWatchPos + new Vector2((ImageStopWatch.width / 2) + SmallHandPos.x,
									(ImageStopWatch.height / 2) + SmallHandPos.y));
		GUI.DrawTexture(new Rect(stopWatchPos.x + SmallHandPos.x, stopWatchPos.y + SmallHandPos.y, ImageStopWatch.width,
									ImageStopWatch.height), ImageHandSmall);

		GUI.matrix = Matrix4x4.TRS(Vector3.zero, Quaternion.identity, Vector3.one);

		// draw watch face
		GUI.color = Color.white;
		GUI.DrawTexture(new Rect(stopWatchPos.x, stopWatchPos.y, ImageStopWatch.width, ImageStopWatch.height), ImageStopWatch);

	}


	/// <summary>
	/// starts the stopwatch (a better name for this would ofcourse have
	/// been 'Start' but that would have conflicted with the standard
	/// Unity 'Start' method
	/// </summary>
	public void Run()
	{

		vp_Timer.Start(m_Timer);
		if(!GetComponent<AudioSource>().isPlaying)
			GetComponent<AudioSource>().Play();

	}


	/// <summary>
	/// stops the stopwatch
	/// </summary>
	public void Stop()
	{

		m_Timer.Cancel();
		if (GetComponent<AudioSource>().isPlaying)
			GetComponent<AudioSource>().Stop();

	}


	/// <summary>
	/// pauses the stopwatch
	/// </summary>
	public void Pause()
	{

		m_Timer.Paused = !m_Timer.Paused;
		if (!GetComponent<AudioSource>().isPlaying && m_Timer.Active && !m_Timer.Paused)
			GetComponent<AudioSource>().Play();
		else if (GetComponent<AudioSource>().isPlaying && (!m_Timer.Active || m_Timer.Paused))
			GetComponent<AudioSource>().Stop();

	}


}
