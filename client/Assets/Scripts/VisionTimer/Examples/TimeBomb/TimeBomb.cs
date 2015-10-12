/////////////////////////////////////////////////////////////////////////////////
//
//	TimeBomb.cs
//
//	description:	this is an example of using multiple timer handles to create
//					more advanced animation behaviors - in this case for a time
//					bomb display that counts down to a pre-blast state before
//					finally exploding. timer handles are used to blink numbers,
//					to vibrate the display depending on timer duration and to
//					determine when to play various sounds
//
/////////////////////////////////////////////////////////////////////////////////

using System.Collections;
using UnityEngine;


class TimeBomb : MonoBehaviour
{

	// variables
	public Vector2 FontPosOffset = new Vector2(14.0f, 13.0f);
	public float FontBackMargin = 8;
	public float FontBackSizeOffset = -29;
	public Vector2 DigitalPosition = new Vector2(0, 0);
	private Vector2 m_StartPosition = new Vector2(0, 0);
	public Color DigitalNumbersColor = Color.red;
	float m_TimeBombNumberAlpha = 1.0f;
	float m_TimeBombFlashAlpha = 0.0f;
	public float BombTime = 11;
	int m_LastSecond = 0;

	// timer handles
	vp_Timer.Handle m_Timer = new vp_Timer.Handle();
	vp_Timer.Handle m_NumberBlinkTimer = new vp_Timer.Handle();
	vp_Timer.Handle m_ExplosionDelayTimer = new vp_Timer.Handle();

	// images
	public Texture ImagePixel = null;
	public Texture ImageDigitalDisplay = null;

	// sounds
	public AudioClip m_ExplosionSound = null;
	public AudioClip m_BlipSound = null;
	public AudioClip m_RumbleFadeInSound = null;
	public AudioClip m_SoftTickLoopSound = null;

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

		m_StartPosition = DigitalPosition;

		// set off a timer to make the bomb explode in a few seconds
		ScheduleBomb(BombTime);

	}


	/// <summary>
	/// 
	/// </summary>
	void OnGUI()
	{

		// only draw the bomb display if atleast one timer is running
		if(m_Timer.Active || m_NumberBlinkTimer.Active || m_ExplosionDelayTimer.Active)
			DrawDigital();

		// draw timebomb flash
		if (m_TimeBombFlashAlpha > 0.0f)
		{
			m_TimeBombFlashAlpha -= (Time.deltaTime * 0.33f);
			GUI.color = new Color(1, 1, 1, m_TimeBombFlashAlpha);
			GUI.DrawTexture(new Rect(0, 0, Screen.width, Screen.height), ImagePixel);
		}

		if (m_Timer.Active)
		{

			// make sure we always play the soft tick sound if timer is active
			if (!GetComponent<AudioSource>().isPlaying)
				GetComponent<AudioSource>().Play();

			// play a louder blip sound every second. this works by checking
			// the number of the last second that we played the sound, and
			// only playing it again when the number changes
			if (m_LastSecond != vp_TimeUtility.TimeToUnits(m_Timer.Duration).seconds)
			{
				GetComponent<AudioSource>().PlayOneShot(m_BlipSound);
				m_LastSecond = vp_TimeUtility.TimeToUnits(m_Timer.Duration).seconds;
			}

		}

		// if we are in the pre-blast state, shake the bomb display, and
		// make the magnitude of the shake correspond to the duration of
		// the explosion delay timer
		if (m_ExplosionDelayTimer.Active)
		{
			DigitalPosition = m_StartPosition;	// this is needed to prevent the display from drifting away
			float maxShakeOffset = Mathf.Max(0, m_ExplosionDelayTimer.Duration * 5.0f);
			Vector2 shake = new Vector2(Random.value * maxShakeOffset, Random.value * maxShakeOffset);
			// inverse the direction in 50% of the cases
			if (Random.value < 0.5f) shake.x = -shake.x;
			if (Random.value < 0.5f) shake.y = -shake.y;
			DigitalPosition += shake;
		}
		

	}


	/// <summary>
	/// draws the digital display for the bomb. the vp_TimeUtility
	/// class is used to convert the vp_Timer duration into a formatted
	/// string showing minutes, seconds and hundredths delimited by ':'
	/// </summary>
	void DrawDigital()
	{

		// draw background for the digital display
		Rect rect = new Rect((Screen.width * 0.5f) + DigitalPosition.x - (ImageDigitalDisplay.width * 0.5f),
							(Screen.height * 0.5f) + DigitalPosition.y - (ImageDigitalDisplay.height * 0.5f),
							(ImageDigitalDisplay.width + FontBackSizeOffset),
							(ImageDigitalDisplay.height + FontBackSizeOffset));

		GUI.color = Color.black;
		GUI.DrawTexture(new Rect(rect.x - FontBackMargin + FontPosOffset.x, rect.y - FontBackMargin + FontPosOffset.y,
								rect.width - 1 + (FontBackMargin * 2) + 1, rect.height - 1 + (FontBackMargin * 2) + 1), ImagePixel);

		// draw digital numbers
		Color col = DigitalNumbersColor;
		col.a = m_TimeBombNumberAlpha;
		GUI.color = col;
		GUI.DrawTexture(new Rect(rect.x + 1 + FontPosOffset.x, rect.y + 2 + FontPosOffset.y, rect.width - 1, rect.height - 1),
						ImagePixel);
		GUI.color = Color.black;
		GUI.Label(new Rect(rect.x + FontPosOffset.x, rect.y + FontPosOffset.y, rect.width, rect.height),
						vp_TimeUtility.TimeToString(m_Timer.DurationLeft, false, true, true, false, true, false),
						DigitalDisplayStyle);

		// draw frame for digital display
		GUI.color = Color.white;
		GUI.DrawTexture(new Rect(rect.x, rect.y, ImageDigitalDisplay.width, ImageDigitalDisplay.height), ImageDigitalDisplay);

	}


	/// <summary>
	/// draws a white, fading out box across the whole screen
	/// </summary>
	void DoTimeBombFlash()
	{

		// draw timebomb flash
		if (m_TimeBombFlashAlpha > 0.0f)
		{
			m_TimeBombFlashAlpha -= (Time.deltaTime * 0.33f);
			GUI.color = new Color(1, 1, 1, m_TimeBombFlashAlpha);
			GUI.DrawTexture(new Rect(0, 0, Screen.width, Screen.height), ImagePixel);
		}

	}


	/// <summary>
	/// sets up the bomb to explode in 'time' seconds, at which point
	/// it enters a brief pre-blast state where the zeroes on the
	/// display blink menacingly as a rumble sound fades in. then
	/// plays an explosion sound and shows the white fullscreen flash
	/// </summary>
	void ScheduleBomb(float time)
	{

		// make the bomb go off in 'time' seconds
		vp_Timer.In(time, delegate()
		{

			// stop the timer tick/blip sounds and start playing
			// a rumble sound
			GetComponent<AudioSource>().Stop();
			GetComponent<AudioSource>().PlayOneShot(m_RumbleFadeInSound);

			// pre-blast state: start blinking the numbers on the
			// bomb display with an interval of 0.33 seconds
			vp_Timer.In(0.33f, delegate()
			{
				if (m_TimeBombNumberAlpha == 0.0f)
					m_TimeBombNumberAlpha = 1.0f;
				else
					m_TimeBombNumberAlpha = 0.0f;
			}, 7, m_NumberBlinkTimer);

			// schedule actual blast, to shortly after the numbers
			// are done blinking
			vp_Timer.In(2.66f, delegate()
			{

				// play the explosion sound
				GetComponent<AudioSource>().Stop();
				GetComponent<AudioSource>().PlayOneShot(m_ExplosionSound);

				// enable the full screen white flash (setting alpha to
				// above 1 will mean it is fully white for more than
				// one second before starting to fade out)
				m_TimeBombFlashAlpha = 2.5f;

				// restore misc values
				m_TimeBombNumberAlpha = 1.0f;
				DigitalPosition = m_StartPosition;

			}, m_ExplosionDelayTimer);
		}, m_Timer);

	}


	/// <summary>
	/// call this when the hero cuts the correct wire in the
	/// nick of time!
	/// </summary>
	void CancelBomb()
	{

		GetComponent<AudioSource>().Stop();
		DigitalPosition = m_StartPosition;
		m_Timer.Cancel();
		m_TimeBombNumberAlpha = 1.0f;
		m_NumberBlinkTimer.Cancel();
		m_ExplosionDelayTimer.Cancel();

	}


}
