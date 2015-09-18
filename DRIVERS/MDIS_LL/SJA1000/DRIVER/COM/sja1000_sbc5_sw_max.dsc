#************************** MDIS4 device descriptor *************************
#
# sja1000_sbc5_sw_max.dsc: Descriptor for SJA1000_SBC5
#
#****************************************************************************

can_sw_1a {

    # ------------------------------------------------------------------------
    #        general parameters (don't modify)
    # ------------------------------------------------------------------------
    DESC_TYPE = U_INT32 0x1
    HW_TYPE = STRING SJA1000

    # ------------------------------------------------------------------------
    #        reference to base board
    # ------------------------------------------------------------------------
    BOARD_NAME = STRING a201_1
    DEVICE_SLOT = U_INT32 0x0

    # ------------------------------------------------------------------------
    #        device parameters
    # ------------------------------------------------------------------------
    SUBDEVICE_OFFSET_0 = U_INT32 0x0

    # CAN controller input clock
    INCLOCK = U_INT32 16000000

    # Physical interface
    # 0 := user defined
    # 1 := RS485
    # 2 := ISO highspeed
    # 3 := passive
    PHYSINTF = U_INT32 2

    # Value of BUS_CFG register when PHYSINTF is set to 0
    PI_USERCFG = U_INT32 0x0

    # CBP comparator bypass function
    # 0 := comparator bypass disabled
    # 1 := comparator bypass enabled
    CBYPASSF = U_INT32 0

    # filter mode
    # 0 := single filter
    # 1 := dual filter
    DUAL_FILTER = U_INT32 0

    # interrupt mask
    # Range: 0x0..0xff
    INTERRUPT_MASK = U_INT32 0x0

    # ------------------------------------------------------------------------
    #        debug levels (optional)
    #        these keys have only effect on debug drivers
    # ------------------------------------------------------------------------
    DEBUG_LEVEL = U_INT32 0xc0008000
    DEBUG_LEVEL_MK = U_INT32 0xc0008000
    DEBUG_LEVEL_OSS = U_INT32 0xc0008000
    DEBUG_LEVEL_DESC = U_INT32 0xc0008000
}
can_sw_1b {

    # ------------------------------------------------------------------------
    #        general parameters (don't modify)
    # ------------------------------------------------------------------------
    DESC_TYPE = U_INT32 0x1
    HW_TYPE = STRING SJA1000

    # ------------------------------------------------------------------------
    #        reference to base board
    # ------------------------------------------------------------------------
    BOARD_NAME = STRING a201_1
    DEVICE_SLOT = U_INT32 0x0

    # ------------------------------------------------------------------------
    #        device parameters
    # ------------------------------------------------------------------------
    SUBDEVICE_OFFSET_0 = U_INT32 0x40

    # CAN controller input clock
    INCLOCK = U_INT32 16000000

    # Physical interface
    # 0 := user defined
    # 1 := RS485
    # 2 := ISO highspeed
    # 3 := passive
    PHYSINTF = U_INT32 2

    # Value of BUS_CFG register when PHYSINTF is set to 0
    PI_USERCFG = U_INT32 0x0

    # CBP comparator bypass function
    # 0 := comparator bypass disabled
    # 1 := comparator bypass enabled
    CBYPASSF = U_INT32 0

    # filter mode
    # 0 := single filter
    # 1 := dual filter
    DUAL_FILTER = U_INT32 0

    # interrupt mask
    # Range: 0x0..0xff
    INTERRUPT_MASK = U_INT32 0x0

    # ------------------------------------------------------------------------
    #        debug levels (optional)
    #        these keys have only effect on debug drivers
    # ------------------------------------------------------------------------
    DEBUG_LEVEL = U_INT32 0xc0008000
    DEBUG_LEVEL_MK = U_INT32 0xc0008000
    DEBUG_LEVEL_OSS = U_INT32 0xc0008000
    DEBUG_LEVEL_DESC = U_INT32 0xc0008000
}
can_sw_1c {

    # ------------------------------------------------------------------------
    #        general parameters (don't modify)
    # ------------------------------------------------------------------------
    DESC_TYPE = U_INT32 0x1
    HW_TYPE = STRING SJA1000

    # ------------------------------------------------------------------------
    #        reference to base board
    # ------------------------------------------------------------------------
    BOARD_NAME = STRING a201_1
    DEVICE_SLOT = U_INT32 0x0

    # ------------------------------------------------------------------------
    #        device parameters
    # ------------------------------------------------------------------------
    SUBDEVICE_OFFSET_0 = U_INT32 0x80

    # CAN controller input clock
    INCLOCK = U_INT32 16000000

    # Physical interface
    # 0 := user defined
    # 1 := RS485
    # 2 := ISO highspeed
    # 3 := passive
    PHYSINTF = U_INT32 2

    # Value of BUS_CFG register when PHYSINTF is set to 0
    PI_USERCFG = U_INT32 0x0

    # CBP comparator bypass function
    # 0 := comparator bypass disabled
    # 1 := comparator bypass enabled
    CBYPASSF = U_INT32 0

    # filter mode
    # 0 := single filter
    # 1 := dual filter
    DUAL_FILTER = U_INT32 0

    # interrupt mask
    # Range: 0x0..0xff
    INTERRUPT_MASK = U_INT32 0x0

    # ------------------------------------------------------------------------
    #        debug levels (optional)
    #        these keys have only effect on debug drivers
    # ------------------------------------------------------------------------
    DEBUG_LEVEL = U_INT32 0xc0008000
    DEBUG_LEVEL_MK = U_INT32 0xc0008000
    DEBUG_LEVEL_OSS = U_INT32 0xc0008000
    DEBUG_LEVEL_DESC = U_INT32 0xc0008000
}
can_sw_1d {

    # ------------------------------------------------------------------------
    #        general parameters (don't modify)
    # ------------------------------------------------------------------------
    DESC_TYPE = U_INT32 0x1
    HW_TYPE = STRING SJA1000

    # ------------------------------------------------------------------------
    #        reference to base board
    # ------------------------------------------------------------------------
    BOARD_NAME = STRING a201_1
    DEVICE_SLOT = U_INT32 0x0

    # ------------------------------------------------------------------------
    #        device parameters
    # ------------------------------------------------------------------------
    SUBDEVICE_OFFSET_0 = U_INT32 0xc0

    # CAN controller input clock
    INCLOCK = U_INT32 16000000

    # Physical interface
    # 0 := user defined
    # 1 := RS485
    # 2 := ISO highspeed
    # 3 := passive
    PHYSINTF = U_INT32 2

    # Value of BUS_CFG register when PHYSINTF is set to 0
    PI_USERCFG = U_INT32 0x0

    # CBP comparator bypass function
    # 0 := comparator bypass disabled
    # 1 := comparator bypass enabled
    CBYPASSF = U_INT32 0

    # filter mode
    # 0 := single filter
    # 1 := dual filter
    DUAL_FILTER = U_INT32 0

    # interrupt mask
    # Range: 0x0..0xff
    INTERRUPT_MASK = U_INT32 0x0

    # ------------------------------------------------------------------------
    #        debug levels (optional)
    #        these keys have only effect on debug drivers
    # ------------------------------------------------------------------------
    DEBUG_LEVEL = U_INT32 0xc0008000
    DEBUG_LEVEL_MK = U_INT32 0xc0008000
    DEBUG_LEVEL_OSS = U_INT32 0xc0008000
    DEBUG_LEVEL_DESC = U_INT32 0xc0008000
}
# EOF
