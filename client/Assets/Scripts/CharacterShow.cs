using UnityEngine;
using System.Collections;

public class CharacterShow : MonoBehaviour
{

    // Use this for initialization
    void Start()
    {

    }

    // Update is called once per frame
    void Update()
    {

    }

    public void OnPress(bool isPress)
    {

        if (isPress)
        {
            StartMenueController.instance.OnCharacteronChaShowPanelClick(gameObject);
        }
    }
}
