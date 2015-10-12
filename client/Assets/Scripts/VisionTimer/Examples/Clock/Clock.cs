/////////////////////////////////////////////////////////////////////////////////
//
//	Clock.cs
//
//	description:	this class shows how to use the vp_TimeUtility methods to set
//					up a regular clock showing the current system time. note that
//					it does not use vp_Timer objects, only the utility class
//
/////////////////////////////////////////////////////////////////////////////////

using UnityEngine;


class Clock : MonoBehaviour
{

	// variables
	public bool DrawDigitalClock = true;
	public bool DrawAnalogClock = true;
	public Vector2 FontPosOffset = new Vector2(14.0f, 13.0f);
	public float FontBackMargin = 8;
	public float FontBackSizeOffset = -29;
	public Vector2 DigitalPosition = new Vector2(-255, -230);
	public Vector2 AnalogPosition = new Vector2(240, 140);
	public Vector2 HandPos = new Vector2(0, 0);
	public Color DigitalNumbersColor = new Color(0.3f, 0.3f, 1, 1.0f);
	int m_LastSecond = 0;

	// images
	public Texture ImagePixel = null;
	public Texture ImagePixelBlack = null;
	public Texture ImageDigitalDisplay = null;
	public Texture ImageClockFace = null;
	public Texture ImageClockHandHour = null;
	public Texture ImageClockHandMinute = null;
	public Texture ImageClockHandSecond = null;


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
	void Start()
	{
		m_LastSecond = vp_TimeUtility.SystemTimeToUnits().seconds;
	}


	/// <summary>
	/// 
	/// </summary>
	void OnGUI()
	{

		if (DrawDigitalClock)
			DrawDigital();

		if(DrawAnalogClock)
			DrawAnalog();

		// play a tick sound every second. this works by checking the
		// number of the last second that we played the sound, and
		// only playing it again when the number changes
		if (m_LastSecond != vp_TimeUtility.SystemTimeToUnits().seconds)
		{
			GetComponent<AudioSource>().Play();
			m_LastSecond = vp_TimeUtility.SystemTimeToUnits().seconds;
		}

	}


	/// <summary>
	/// draws the digital display for the clock. the vp_TimeUtility
	/// class is used to convert current system time into a formatted
	/// string showing hours, minutes and seconds delimited by ':'
	/// </summary>
	void DrawDigital()
	{

		// format time strings
		string digitalDisplayTimeString = "";

		digitalDisplayTimeString = vp_TimeUtility.SystemTimeToString(true, true, true, false, false, false);

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
	/// draws the analog display for the clock. the vp_TimeUtility
	/// class is used to convert current system time into angles for
	/// the clock hands
	/// </summary>
	void DrawAnalog()
	{

		// prepare for drawing analog watch
		GUI.color = Color.white;
		Vector2 stopWatchPos = new Vector2(((Screen.width * 0.5f) + AnalogPosition.x) - (ImageClockFace.width * 0.5f),
											((Screen.height * 0.5f) + AnalogPosition.y) - (ImageClockFace.height * 0.5f));
		Vector3 clockAngle = Vector3.zero;

		// determine angles for clock hands
		clockAngle = vp_TimeUtility.SystemTimeToDegrees(false);		// make minutes and seconds snap
		clockAngle.x = vp_TimeUtility.SystemTimeToDegrees().x;		// make hours move smooth

		// draw seconds clock hand
		GUIUtility.RotateAroundPivot(clockAngle.z, stopWatchPos + new Vector2((ImageClockFace.width * 0.5f) + HandPos.x,
									(ImageClockFace.height * 0.5f) + HandPos.y));
		GUI.DrawTexture(new Rect(stopWatchPos.x + HandPos.x, stopWatchPos.y + HandPos.y, ImageClockFace.width,
									ImageClockFace.height), ImageClockHandSecond);
		GUI.matrix = Matrix4x4.TRS(Vector3.zero, Quaternion.identity, Vector3.one);

		// draw hours clock hand
		GUIUtility.RotateAroundPivot(clockAngle.x, stopWatchPos + new Vector2((ImageClockFace.width * 0.5f) + HandPos.x,
									(ImageClockFace.height * 0.5f) + HandPos.y));
		GUI.DrawTexture(new Rect(stopWatchPos.x + HandPos.x, stopWatchPos.y + HandPos.y, ImageClockFace.width,
									ImageClockFace.height), ImageClockHandHour);
		GUI.matrix = Matrix4x4.TRS(Vector3.zero, Quaternion.identity, Vector3.one);

		// draw minutes clock hand
		GUIUtility.RotateAroundPivot(clockAngle.y, stopWatchPos + new Vector2((ImageClockFace.width * 0.5f) + HandPos.x,
									(ImageClockFace.height * 0.5f) + HandPos.y));
		GUI.DrawTexture(new Rect(stopWatchPos.x + HandPos.x, stopWatchPos.y + HandPos.y, ImageClockFace.width,
									ImageClockFace.height), ImageClockHandMinute);
		GUI.matrix = Matrix4x4.TRS(Vector3.zero, Quaternion.identity, Vector3.one);

		// draw clock face
		GUI.color = Color.white;
		GUI.DrawTexture(new Rect(stopWatchPos.x, stopWatchPos.y, ImageClockFace.width, ImageClockFace.height), ImageClockFace);

	}


}
