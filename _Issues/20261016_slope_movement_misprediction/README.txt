g.4.odd

    gametic 35763   (% 70 == 63)

    the latest snapshot from the server has position
        x -29647071
        y -94114635
        z -3381821

        server tic          35998
        lastEchoedClientTic 35758
        p.tic               35758

    This happens to match the cl_savedsnaps[59]  (lastEchoedClientTic + 1).





in a good-prediction mode,
    gametic 37794  ( % 70 == 64)

    latest snapshot

        x 2369164
        y 57053820
        z 0

        server tic 38029
        lastEchoedClientTic 37789
        p.tic               37788

    The snapshot matches cl_savedsnaps[59] (lastEchoedTic)

    This seems normal, and the off-by-one in the bad prediction is the cause of the misprediction..?



		player.client.messenger->Reliable().Write (
		             SVC_LoadMap(::wadfiles, ::patchfiles, d_mapname.c_str(), 0));

		SV_ExpectResourceDigests(player.client);


g.5.odd


tic 3520:
    first mispredicting tic

    Only mo->z disagrees when comparing prevsnap to correctedprevsnap

    corrected comes out to z == 349651,  prevsnap (i.e. player then-current state) == 349645
    wow so close!

    prevsnap values for z for tic:

    3513:   0
    3514:   0
    3515:   0
    3516:   0
    3517:   0
    3518:   544
    3519:   7252
    3520:   349645

    Stepping through 3520's prediction iterations:
        0
        544
        7252
        349651

    The next few tics:
    gametic     server snapshot tic     server snapshot z
    3522        3747                    544
    3523        3748                    7252
    3524        3749                    349651

    The current state going into tic 3520 was WRONG!


    .... 3520...  The NetdemoCap processing set z to 349645

    !!!!!!!!!!!!!   If I remove the logic that forces the player mobj in CL_NetdemoCap, then
                    prediction goes perfectly in the netdemo!


g.7.odd

    playing back without the mo overwrite.

    gametic 487 - first logged misprediction.
        The saved snapshot history is already wrong on the z values, but only by small amounts initially.


g.8.odd

    added P_CheckSlopeWalk plane->c change detection, got consistent results between live and netdemo,
    with the exception of the first sample:

    live:                                               netdemo:
        [10:01:37] c changed from 0 to 65228:         [10:06:44] c changed from 65421 to 65228:
        [10:01:37] c changed from 65228 to 65223:     [10:06:45] c changed from 65228 to 65223:
        [10:01:37] c changed from 65223 to 65228:     [10:06:45] c changed from 65223 to 65228:
        [10:01:37] c changed from 65228 to 65223:     [10:06:45] c changed from 65228 to 65223:
        [10:01:38] c changed from 65223 to 65228:     [10:06:45] c changed from 65223 to 65228:
        [10:01:38] c changed from 65228 to 65223:     [10:06:45] c changed from 65228 to 65223:
        [10:01:38] c changed from 65223 to 65228:     [10:06:45] c changed from 65223 to 65228:
        [10:01:38] c changed from 65228 to 65223:     [10:06:45] c changed from 65228 to 65223:
        [10:01:38] c changed from 65223 to 65228:     [10:06:45] c changed from 65223 to 65228:
        [10:01:38] c changed from 65228 to 65223:     [10:06:45] c changed from 65228 to 65223:
        [10:01:38] c changed from 65223 to 64978:     [10:06:45] c changed from 65223 to 64978:
        [10:01:38] c changed from 64978 to 65223:     [10:06:45] c changed from 64978 to 65223:
        [10:01:38] c changed from 65223 to 64978:     [10:06:45] c changed from 65223 to 64978:
        [10:01:38] c changed from 64978 to 65223:     [10:06:45] c changed from 64978 to 65223:
        [10:01:38] c changed from 65223 to 64978:     [10:06:45] c changed from 65223 to 64978:
        [10:01:38] c changed from 64978 to 65223:     [10:06:45] c changed from 64978 to 65223:
        [10:01:38] c changed from 65223 to 64978:     [10:06:45] c changed from 65223 to 64978:
        [10:01:38] c changed from 64978 to 64979:     [10:06:45] c changed from 64978 to 64979:
        [10:01:38] c changed from 64979 to 64978:     [10:06:45] c changed from 64979 to 64978:
        [10:01:38] c changed from 64978 to 64979:     [10:06:45] c changed from 64978 to 64979:
        [10:01:38] c changed from 64979 to 64978:     [10:06:45] c changed from 64979 to 64978:
        [10:01:38] c changed from 64978 to 64979:     [10:06:45] c changed from 64978 to 64979:
        [10:01:38] c changed from 64979 to 64978:     [10:06:45] c changed from 64979 to 64978:
        [10:01:38] c changed from 64978 to 64979:     [10:06:45] c changed from 64978 to 64979:
        [10:01:38] c changed from 64979 to 64978:     [10:06:45] c changed from 64979 to 64978:
        [10:01:38] c changed from 64978 to 64979:     [10:06:45] c changed from 64978 to 64979:
        [10:01:39] c changed from 64979 to 64975:     [10:06:46] c changed from 64979 to 64975:
        [10:01:41] c changed from 64975 to 65421:     [10:06:48] c changed from 64975 to 65421:

    HOWEVER!

        None showed up during prediction in either case!  Why not?  isn't P_CheckSlopeWalk involved?
