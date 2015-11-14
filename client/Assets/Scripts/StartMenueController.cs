using UnityEngine;
using System.Collections;


public class StartMenueController : MonoBehaviour
{
    public static StartMenueController instance = null;
    public TweenScale startPannelTween;
    public TweenScale loginPannelTween;
    public TweenScale registerPannelTween;
    public TweenScale gameZonePannelTween;
    public TweenPosition chaSelectionPannelTween;
    public TweenPosition chaShowPannelTween;

    public UIInput userNameInputonLoginPannel;
    public UIInput userPwdInputonLoginPannel;

    public UIInput userNameInputonRegisterPannel;
    public UIInput userPwdInputonRegisterPannel;
    public UIInput userPwdConfirmInputonRegisterPannel;

    public UILabel userNameonStartPannel;
    public UILabel zoneNameonStartPannel;

    public static string userName = "";
    public static string userPwd = "";

    public static string gameZone = "";
    private bool hasInitGamezonelist = false;
    public UIGrid gameZoneList;
    public GameObject hostBnt;
    public GameObject normalBnt;

    public GameZoneBntProperty currGzp;
    public GameObject currGo;

    public GameObject[] characterArray;
    public GameObject currCharacter = null;

    public GameObject[] selectedCharacterArray;
    public Transform currCharacterPos;
    public UILabel name;
    public UILabel level;
    public UIInput nameInput;
    // Use this for initialization

    void Awake()
    {
        instance = this;
    }

    void Start()
    {
        InitGameZonesList();
    }

    // Update is called once per frame
    void Update()
    {

    }
    public void OnUserNameClick()
    {
        /// hide the start pannel
        startPannelTween.PlayForward();
        StartCoroutine(HidePannel(startPannelTween.gameObject));
        /// show LoginPannel 
        ShowPannel(loginPannelTween.gameObject);
        loginPannelTween.PlayForward();
    }

    IEnumerator HidePannel(GameObject go)
    {
        yield return new WaitForSeconds(0.5f);
        go.SetActive(false);
    }

    private void ShowPannel(GameObject go)
    {
        go.SetActive(true);
    }

    /// @todo1 choose server
    public void OnUserServerZoneClick()
    {
        /// @todo1 choose server

        /// first hide start pannel
        startPannelTween.PlayForward();
        StartCoroutine(HidePannel(startPannelTween.gameObject));

        /// then show game zone  pannel
        ShowPannel(gameZonePannelTween.gameObject);
        gameZonePannelTween.PlayForward();

        /// init list
        InitGameZonesList();
    }

    /// @ connect to server and verify the user
    public void OnStartPanlEnterGameBntGameClick()
    {
        /// @ connect to server and verify the user
        /// todo1
        /// 
        /// jump to  the character-selection scene
        /// first hide cha selection pannel
        startPannelTween.gameObject.SetActive(false);

        /// then show character pannel
        ShowPannel(chaSelectionPannelTween.gameObject);
        chaSelectionPannelTween.PlayForward();
    }

    public void OnChaSelctionPanlChaneAnotherBntGameClick()
    {
        /// jump to  the character-show scene
        /// 
        /// first hide cha selection pannel
        chaSelectionPannelTween.PlayReverse();
        StartCoroutine(HidePannel(chaSelectionPannelTween.gameObject));

        /// then show show pannel
        ShowPannel(chaShowPannelTween.gameObject);
        chaShowPannelTween.PlayForward();
    }

    public void OnLoginPannelLoginBntClick()
    {
        /// get user anme and pwd
        userName = userNameInputonLoginPannel.value;
        userPwd = userPwdInputonLoginPannel.value;
        /// return to StartPannel 
        loginPannelTween.PlayReverse();
        StartCoroutine(HidePannel(loginPannelTween.gameObject));
        ShowPannel(startPannelTween.gameObject);
        startPannelTween.PlayReverse();

        userNameonStartPannel.text = userName;
    }

    public void OnLoginPannelCloseBntClick()
    {
        ///  return to StartPannel 
        loginPannelTween.PlayReverse();
        StartCoroutine(HidePannel(loginPannelTween.gameObject));
        ShowPannel(startPannelTween.gameObject);
        startPannelTween.PlayReverse();
    }

    public void OnLoginPannelRegisterBntClick()
    {
        /// first hide login pannel
        loginPannelTween.PlayReverse();
        StartCoroutine(HidePannel(loginPannelTween.gameObject));

        /// then show register pannel
        ShowPannel(registerPannelTween.gameObject);
        registerPannelTween.PlayForward();
    }

    /// 1 @todo client-side verification and server-side verification
    /// 2 @todo connect fails
    public void OnRegisterPannelRegisterandLoginBntClick()
    {
        /// 1 @todo client-side verification and server-side verification
        /// 2 @todo connect fails

        /// 3 connect succeed, store username and pwd, return to start pannel
        userName = userNameInputonRegisterPannel.value;
        userPwd = userPwdInputonRegisterPannel.value;

        /// first hide register  pannel
        registerPannelTween.PlayReverse();
        StartCoroutine(HidePannel(registerPannelTween.gameObject));

        /// then show start pannel
        ShowPannel(startPannelTween.gameObject);
        startPannelTween.PlayReverse();

        userNameonStartPannel.text = userName;
    }

    public void OnRegisterPannelCancelBntClick()
    {
        /// first hide register  pannel
        registerPannelTween.PlayReverse();
        StartCoroutine(HidePannel(registerPannelTween.gameObject));

        /// then show start pannel
        ShowPannel(startPannelTween.gameObject);
        startPannelTween.PlayReverse();
    }

    public void InitGameZonesList()
    {
        if (hasInitGamezonelist) return;
        /// 1. connect to server and get the game zone list infos'
        /// @TODO
        /// 2. add game zones based on the infos from server
        string ip;
        string name;
        int count;
        GameObject go;
        GameZoneBntProperty gzp;

        for (int i = 0; i < 20; i++)
        {
            ip = "127.0.0.1:9080";
            name = "Zone " + i + ", Apple";
            count = Random.Range(0, 100);

            if (count > 50)
            {
                /// hot
                go = NGUITools.AddChild(gameZoneList.gameObject, hostBnt);

            }
            else
            {
                /// normal
                go = NGUITools.AddChild(gameZoneList.gameObject, normalBnt);
            }

            gzp = go.GetComponent<GameZoneBntProperty>();
            gzp.Name = name;
            gzp.Ip = ip;

            gameZoneList.AddChild(go.transform);
        }

        hasInitGamezonelist = true;
    }

    public void OnSelectZone(GameObject go)
    {
        currGzp = currGo.GetComponent<GameZoneBntProperty>();

        currGo.GetComponent<UISprite>().spriteName = go.GetComponent<UISprite>().spriteName;

        currGo.transform.GetChild(1).GetComponent<UISprite>().spriteName = go.transform.GetChild(1).GetComponent<UISprite>().spriteName;

        currGo.transform.GetChild(1).GetChild(0).GetComponent<UILabel>().text =
            go.transform.GetChild(1).GetChild(0).GetComponent<UILabel>().text;

        currGo.transform.GetChild(0).GetComponent<UILabel>().text =
    go.transform.GetChild(0).GetComponent<UILabel>().text;

    }

    public void OnSelectedZoneBntClicked()
    {
        /// hide game zone pannel
        gameZonePannelTween.PlayReverse();
        StartCoroutine(HidePannel(gameZonePannelTween.gameObject));
        /// show start pannel
        ShowPannel(startPannelTween.gameObject);
        startPannelTween.PlayReverse();

        ///update sozne name on start pannel
        if (zoneNameonStartPannel == null) Debug.Log("null\n");
        if (currGzp == null) Debug.Log(" currGzp null\n");
        currGzp = currGo.GetComponent<GameZoneBntProperty>();
        zoneNameonStartPannel.text = currGo.transform.GetChild(0).GetComponent<UILabel>().text;

    }

    public void OnCharacteronChaShowPanelClick(GameObject go)
    {
        if (go.tag == "man")
        {
            iTween.ScaleTo(go, new Vector3(22f, 22f, 22f), 0.5f);
        }
        else
        {
            iTween.ScaleTo(go, new Vector3(27f, 27f, 27f), 0.5f);
        }
        if (currCharacter != null && currCharacter != go)
        {
            if (go.tag == "women")
            {
                iTween.ScaleTo(currCharacter, new Vector3(17f, 17f, 17f), 0.5f);
            }
            else
            {
                iTween.ScaleTo(currCharacter, new Vector3(20f, 20f, 20f), 0.5f);
            }
        }
        currCharacter = go;
    }

    // justify if  the input name has exsited in  server database 
    //@todo
    // if user slectes character
    //#todo
    public void OnChacterShowBntClicked()
    {
        if (nameInput.value == "") return;

        // justify if  the input name has exsited in  server database 
        //@Todo
        // if user slectes character
        //#todo

        int index = -1;
        for (int i = 0; i < characterArray.Length; i++)
        {
            if (currCharacter == characterArray[i])
            {
                index = i;
                break;
            }
        }
        if (index == -1)
        {
            return;
        }
        Debug.Log("hello");
        // destroy the esisting character
        if (currCharacterPos.childCount != 0)
            GameObject.Destroy(currCharacterPos.GetChild(0).gameObject);

        GameObject go =
            GameObject.Instantiate(selectedCharacterArray[index], Vector3.zero,
            Quaternion.identity) as GameObject;
        go.transform.parent = currCharacterPos;
        if (go.tag == "man")
            go.transform.localScale = new Vector3(0.85f, 0.85f, 0.85f);
        else
            go.transform.localScale = new Vector3(1f, 1f, 1f);
        go.transform.localRotation = Quaternion.identity;
        go.transform.localPosition = Vector3.zero;

        // update name and level
        name.text = nameInput.value;
        level.text = "Lv. 0";
        GotoCharaterSlect();
    }

    public void GotoCharaterSlect()
    {
        // hide current screen
        chaShowPannelTween.PlayReverse();
        StartCoroutine(HidePannel(chaShowPannelTween.gameObject));

        // show the cha slect scene
        ShowPannel(chaSelectionPannelTween.gameObject);
        chaSelectionPannelTween.PlayForward();
    }
}
