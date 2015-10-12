using UnityEngine;
using System.Collections;

public class GameZoneBntProperty : MonoBehaviour
{
	private UILabel label;

	private string ip = "localhost:36000";
	public string Ip
	{
		get { return ip; }
		set { ip = value; }
	}

	private string name;
	public string Name
	{
		get { return name; }
		set
		{
			name = value;
			if (label == null)
				label = gameObject.transform.GetChild(0).GetComponent<UILabel>();
			label.text = name;
		}
	}

	private int onlineCount = 100;
	public int OnlineCount
	{
		get { return onlineCount; }
		set { onlineCount = value; }
	}

	public void OnPress(bool isPress)
	{
		if (isPress == false)
		{
			//选择了当前的服务器
			transform.root.SendMessage("OnSelectZone", this.gameObject);
		}
	}

	// Use this for initialization
	void Start() { }

	// Update is called once per frame
	void Update() { }
}
